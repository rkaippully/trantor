/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

/*
  Boot sector code present in MBR of a fixed drive.
*/

  .struct 0
pentry_start:
pentry_status:
  .struct pentry_status+1
pentry_start_head:
  .struct pentry_start_head+1
pentry_start_sec:
  .struct pentry_start_sec+1
pentry_start_cylinder:
  .struct pentry_start_cylinder+1
pentry_part_type:
  .struct pentry_part_type+1
pentry_end_head:
	.struct pentry_end_head+1
pentry_end_sec:
	.struct pentry_end_sec+1
pentry_end_cylinder:
	.struct pentry_end_cylinder+1
pentry_lba_start_low:
  .struct pentry_lba_start_low+2
pentry_lba_start_high:
  .struct pentry_lba_start_high+2
pentry_sec_count:
  .struct pentry_sec_count+4
pentry_end:

.arch i186
.code16
.text

#include "common.S"

/*
  The code in this file is linked at address 0x500 but initially loaded
	elsewhere. So it is important that we make only near memory references till we
	relocate.
*/
.equ RELOC_ADDR, 0x500
.equ NUM_PARTITIONS, 4
.equ PENTRY_LEN, pentry_end - pentry_start
.equ LOAD_ADDR, 0x7c00

.global start
start:
  relocate

  movw  $ptbl_start, %si
  movw  $NUM_PARTITIONS, %cx

  // Check if this entry is active and bootable
1:
  cmpb  $0x80, pentry_status(%si)
  je    active_partition
  // Inactive partition, go to the next one
  addw  $PENTRY_LEN, %si
  loop  1b
	jmp   no_bootable_partition

active_partition:
  // Does this drive support LBA reads?
  movw  $0x55aa, %bx
  movb  $0x41, %ah
  int   $0x13
  jc    no_lba_support
  cmpw  $0xaa55, %bx
  jne   no_lba_support

read_lba:
	movw  pentry_lba_start_low(%si), %cx
	movw  %cx, disk_address_packet+8
	movw  pentry_lba_start_high(%si), %cx
	movw  %cx, disk_address_packet+10

	movw  $5, %cx   // Retry count for read
  movw  $disk_address_packet, %si
1:
  movb  $0x42, %ah
  int   $0x13
  jnc   run_system
  xorb  %ah, %ah
  int   $0x13
  loop  1b
read_fail:
	movw  $disk_read_error_msg, %si
	jmp   print_msg

run_system:
  ljmp  $0, $LOAD_ADDR

no_bootable_partition:
	movw  $no_bootable_partition_msg, %si
	//jmp   print_msg

  def_print_msg

no_lba_support:
	movw  $no_lba_support_msg, %si
	jmp   print_msg


no_bootable_partition_msg:
	.asciz "No bootable partition found."

disk_read_error_msg:
	.asciz "Disk read error."

no_lba_support_msg:
  .asciz "Disk does not support LBA mode."

disk_address_packet:
	.byte   0x10              // Size of packet
	.byte   0                 // Reserved
	.short  1                 // Number of blocks to transfer
	.short  0                 // Offset
	.short  (LOAD_ADDR >> 4)  // Segment
	.short  0                 // LBA block number (short 1)
	.short  0                 // LBA block number (short 2)
	.short  0                 // LBA block number (short 3)
	.short  0                 // LBA block number (short 4)

// Partition table starts here
. = 0x1be

ptbl_start:
.zero NUM_PARTITIONS * PENTRY_LEN

// Boot signature
.short 0xaa55
