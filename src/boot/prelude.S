/*
	Trantor Operating System

	Copyright (C) 2017 Raghu Kaippully
*/

/*
  This file contains code tacked at the start of trantor.sys. It helps load the
  kernel COFF format and set up protected mode.
*/

.arch i486
.code16
.section ".prelude.16", "rx"

#include "common.S"

.equ SMAP_SIG, 0x534d4150
/*
	The memory map is stored at 0x504. Each entry has base address (64 bit),
  length (64 bit), and type (32 bit). We assume a maximum of 25 such entries.
	The total length of mmap in bytes is stored at 0x500 (32 bit).
*/
.equ MMAP_COUNT, 0x500
.equ MMAP_START, 0x504
.equ MMAP_STRUCT_SIZE, 20
.equ MMAP_END, 0x700-0x20

.equ CODE_SEL, 0x0008
.equ DATA_SEL, 0x0010

.global start
start:
	// Detect the amount of memory we have
	xorw  %ax, %ax
	movw  %ax, %es
	movl  $MMAP_START, %edi
	xorl  %ebx, %ebx
next_mmap:
  movl  $SMAP_SIG, %edx
  movl  $MMAP_STRUCT_SIZE, %ecx
  movl  $0xe820, %eax
  int   $0x15
  jc    mem_detect_fail
  cmpl  $SMAP_SIG, %eax
  jne   mem_detect_fail
  cmpl  $0, %ebx
  jne   1f
  subw  $(MMAP_START - MMAP_STRUCT_SIZE), %di
  movl  %edi, MMAP_COUNT
  jmp   2f
1:
  addw  $MMAP_STRUCT_SIZE, %di
  cmpw  $MMAP_END, %di
  jle   next_mmap
  jmp   too_many_mmaps
2:

  // Let us go to protected mode
  cli

  // Enable A20 gate
  inb   $0x92, %al
  orb   $2, %al
  outb  %al, $0x92

  lgdtl gdt_desc
  movl  %cr0, %eax
  orb   $1, %al
  movl  %eax, %cr0
  ljmp  $CODE_SEL, $start32

mem_detect_fail:
  movw  $mem_detect_fail_msg, %si
  //jmp   print_msg

  def_print_msg

too_many_mmaps:
  movw  $too_many_mmaps_msg, %si
  jmp   print_msg

mem_detect_fail_msg:
  .asciz "Could not detect amount of memory."

too_many_mmaps_msg:
  .asciz "Too many memory map entries."

gdt_desc:
  .short  8*3-1                 // limit (3 descriptors)
  .long   gdt                   // base

.align 3
gdt:
  .quad 0x0000000000000000
  .quad 0x00cf9a000000ffff      // 4 GB code segment
  .quad 0x00cf92000000ffff      // 4 GB data segment



.arch i486
.code32
.section ".prelude.32", "rx"

// 32-bit initialization
start32:
  movw  $DATA_SEL, %ax
  movw  %ax, %ds
  movw  %ax, %es
  movw  %ax, %fs
  movw  %ax, %gs
  movw  %ax, %ss
  movl  $0x7c00, %esp

1:
  movw  $0x7c42, 0xb8000
  hlt
  jmp   1b
