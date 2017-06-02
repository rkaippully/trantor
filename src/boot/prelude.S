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
	The number of mmap entries is stored at 0x500 (32 bit).
*/
.equ MMAP_COUNT, 0x500
.equ MMAP_START, 0x504
.equ MMAP_STRUCT_SIZE, 20
.equ MMAP_END, 0x700-0x20

/*
  There are two sets of selectors. The first two have a base address of 0. The
  next two have a base address of 0x40100000, so any linear address above
  0xc0000000 will wrap around to 0x00100000 (1 MB).
*/
.equ CODE_SEL,      0x0008
.equ DATA_SEL,      0x0010
.equ WRAP_CODE_SEL, 0x0018
.equ WRAP_DATA_SEL, 0x0020

.global start
start:
  // Clear the screen
  movw  $0x0003,  %ax
  int   $0x10
  movw  $0xb800, %ax
  movw  %ax, %es
  movw  $0x0720, %ax
  movw  $(80*25), %cx
  xorw  %di, %di
  cld
  rep stosw %ax, %es:(%di)

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
  // We reached end of mmap
  subw  $(MMAP_START - MMAP_STRUCT_SIZE), %di
  movw  %di, %ax
  movb  $MMAP_STRUCT_SIZE, %cl
  divb  %cl
  andl  $0xff, %eax
  movl  %eax, MMAP_COUNT
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
  .short  gdt_end-gdt-1         // limit
  .long   gdt                   // base

.align 3
gdt:
  .quad 0x0000000000000000
  .quad 0x00cf9a000000ffff      // 4 GB code segment, base = 0x00000000
  .quad 0x00cf92000000ffff      // 4 GB data segment, base = 0x00000000
  .quad 0x40cf9a100000ffff      // 4 GB code segment, base = 0x40100000
  .quad 0x40cf92100000ffff      // 4 GB data segment, base = 0x40100000
gdt_end:


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

  /*
    Now that we are in protected mode, relocate the kernel just above 1MB in
    physical RAM. The kernel is in DJGPP COFF format. Load it appropriately.

    We assume the kernel to be in a very specific format and quit loading if
    those assumptions are incorrect
  */
reloc_kernel:
  movl  $kimg_start, %esi       // kimg_start is defined in the linker script

  // Validate COFF file header
  cmpw  $0x014c, (%esi)         // COFF magic
  jne   bad_coff_magic
  cmpw  $3, 2(%esi)
  jne   bad_section_count
  cmpw  $0x001c, 16(%esi)       // Size of optional header
  jne   bad_opthdr_size
  cmpw  $0x010f, 18(%esi)       // COFF header flags
  jne   bad_hdr_flags

  // Validate optional header
  cmpw  $0x010b, 20(%esi)       // Magic
  jne   bad_opthdr_magic
  movl  36(%esi), %eax
  movl  %eax, coff_entry_point

  // Read three sections
  movl  $48, %edx
  call  read_section
  movl  $88, %edx
  call  read_section
  movl  $128, %edx
  call  read_section

  movl  coff_entry_point, %ebx
  // Load the "wraparound" segment selectors now
  movw  $WRAP_DATA_SEL, %ax
  movw  %ax, %ds
  movw  %ax, %es
  movw  %ax, %fs
  movw  %ax, %gs

  // Jump to kernel entry point
  pushl $WRAP_CODE_SEL
  pushl %ebx
  lret

  // We should never reach here
  jmp   halt

read_section:
  movl  8(%edx,%esi,), %edi     // Base address of section
  movl  16(%edx,%esi,), %ecx    // Section size in bytes
  cmpl  $0x80, 36(%edx,%esi,)
  je    load_bss
  cmpl  $0x20, 36(%edx,%esi,)
  je    load_section
  cmpl  $0x40, 36(%edx,%esi,)
  jne   bad_section_type
load_section:
  pushl %esi
  addl  20(%edx,%esi,), %esi    // Pointer to section data in COFF file
  rep movsb %ds:(%esi), %es:(%edi)
  popl  %esi
  ret
load_bss:
  xorb  %al, %al
  rep stosb %al, %es:(%edi)
  ret

bad_coff_magic:
  movl  $bad_coff_magic_msg, %esi
  // jmp print_msg_32

print_msg_32:
  movb  $0x07, %ah
  movl  $0xb8000, %edi
1:
  lodsb %ds:(%esi), %al
  cmpb  $0, %al
  je    halt
  stosw %ax, %es:(%edi)
  jmp   1b

halt:
  hlt
  jmp   1b

bad_section_count:
  movl  $bad_section_count_msg, %esi
  jmp   print_msg_32

bad_opthdr_size:
  movl  $bad_opthdr_size_msg, %esi
  jmp   print_msg_32

bad_hdr_flags:
  movl  $bad_hdr_flags_msg, %esi
  jmp   print_msg_32

bad_opthdr_magic:
  movl  $bad_opthdr_magic_msg, %esi
  jmp   print_msg_32

bad_section_type:
  movl  $bad_section_type_msg, %esi
  jmp   print_msg_32

bad_coff_magic_msg:
  .asciz "Incorrect COFF header magic in kernel"
bad_section_count_msg:
  .asciz "Incorrect COFF section count in kernel"
bad_opthdr_size_msg:
  .asciz "Incorrect COFF optional header size in kernel"
bad_hdr_flags_msg:
  .asciz "Incorrect COFF file header flags in kernel"
bad_opthdr_magic_msg:
  .asciz "Incorrect COFF optional header magic in kernel"
bad_section_type_msg:
  .asciz "Unknown COFF section type in kernel"

// Linear address at which execution begins
coff_entry_point:
  .int  0
