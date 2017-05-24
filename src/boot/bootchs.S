/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

/*
  Boot sector code present in floppies and first sector of a bootable partition
  on a FAT volume that only supports CHS addressing.
*/

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
.equ KERNEL_LOAD_SEG, (RELOC_ADDR+512)>>4

.global start
start:
  jmp after_bpb
  nop

  def_bpb

after_bpb:
  relocate

  // Search for valid Trantor boot header
  movw  $0x5254, %ax  /* 0x5254="TR" */
  movw  $tbh, %di
  movw  $3, %cx
  repe scasw %es:(%di), %ax
  jne   bad_system_disk
	movw  $kernel_loc_map, %si
  cmpb  $TBH_VERSION, -2(%si)
  jne   bad_system_disk
  cmpb  $0, (%si)          // kernel_loc_map
  je    bad_system_disk

  cmpb  $0, -1(%si)        // kernel_loc_map_format == CHS
  jne   bad_system_disk

	movw  $KERNEL_LOAD_SEG, %bx
	movw  %bx, %es
	pushw $5      // Retry count
	movw  %sp, %bp

read_sectors:
	// Entry at kernel_loc_map is compatible with int 0x13/ah=0x02
	movb  0(%si), %al
	cmpb  $0, %al
	jz    read_complete
	movw  1(%si), %cx
	movb  3(%si), %dh
	xorw  %bx, %bx
	movb  $0x02, %ah
	int   $0x13
	jc    read_error

read_success:
  /* increment the segment address */
  movw  bpb_bytes_per_sec, %bx
  shrw  $4, %bx       // bx=# of seg pages per sector
  movb  0(%si), %cl
  xorb  %ch, %ch      // cx=number of sectors read
  movw  %es, %ax      // ax=segment address
1:
  addw  %bx, %ax
  loopw 1b
  movw  %ax, %es

  /* increment the TBH entry pointer */
  addw  $4, %si
  movw  $5, (%bp)   // Retry count
  jmp   read_sectors

read_error:
  andw  $0xffff, (%bp)
  jz    read_fail
  decw  (%bp)     // Retry count
  xorb  %ah, %ah
  int   $0x13     // Reset disk drive
  jmp   read_sectors

read_complete:
  ljmp  $KERNEL_LOAD_SEG, $0

read_fail:
  movw  $disk_read_error_msg, %si
  jmp   print_msg

bad_system_disk:
	movw  $bad_system_disk_msg, %si
	//jmp   print_msg

  def_print_msg

bad_system_disk_msg:
  .asciz "Invalid system disk."

disk_read_error_msg:
  .asciz "Disk read error."

/*
	Trantor boot header

	This is an area present only in the boot sector of a Trantor bootable disk.
	This contains information to load the Trantor kernel without understanding
	the underlying file system format.
*/
.align 2  // 4 byte aligned
tbh:

.equ TBH_VERSION, 0     // Expected TBH version

tbh_sig:
	.ascii "TRTRTR"       // Trantor boot header signature
tbh_ver:
	.byte 0	              // Version number of boot header format
kernel_loc_map_format:
  .byte 0               // Format of kernel_loc_map 0x00=CHS, 0x01=LBA
kernel_loc_map:
/*
  Here you will find a list of entries that points to the disk locations where
  kernel is located. Based on kernel_loc_map_format, the entries will have
  either a CHS format or LBA format.

  CHS format:

  byte 0x00 (1 byte) = number of contiguous sectors to read
  byte 0x01 (1 byte) = sector number (1-63) (bits 0-5), high 2 bits off cylinder (bits 6-7)
  byte 0x02 (1 byte) = low 8 bits of cylinder number
  byte 0x03 (1 byte) = head number

  LBA format:

  byte 0x00 (2 bytes) = number of contiguous blocks to read
  byte 0x02 (4 bytes) = starting absolute block number

  The last entry will be followed by a short 0x00.
*/

. = 510

boot_signature:
.short 0xaa55
