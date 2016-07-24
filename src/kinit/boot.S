/*
  Trantor Operating System

  Copyright (C) 2016 Raghu Kaippully
*/


/*
  All processes in trantor have a user space and kernel space. The user space is
	located at linear address 0x00100000 and the kernel space at 0xff000000. The
	user space runs at ring 3 and kernel space at ring 0.

  The kinit process is loaded by a multiboot loader and starts execution at the
	_start symbol. The image has two parts - kinit and kernel. kinit is loaded at
	linear address 0x00100000 and kernel at 0xff000000. Now this may seem to mean
	that kinit is running in user space but it is not. The entire code is running
	at ring 0. This makes it easy to do all the initialization that needs ring 0.
	At the end of initialization, kinit will stop execution and only the kernel
	will remain in memory. The kernel is mapped at 0xff000000 in the linear
	address space of all processes including kinit.
*/

/* Convert a linear address to physical address. This is needed till we enable paging */
#define physical_addr_of(x) ((x) - kernel_start + kinit_end)
.macro physical_addr_of addr, reg
  movl  \addr, \reg
  subl  $kernel_start, \reg
  addl  $kinit_end, \reg
.endm

#define PRESENT     1
#define READ_WRITE  2

/* Create a page directory/table entry for the specified address and store it in reg */
.macro setup_paging_entry addr, reg
  movl  \addr, \reg
  andl  $0xfffff000, \reg
  addl  $(PRESENT + READ_WRITE), \reg
.endm

#define KERNEL_CODE 0x08
#define KERNEL_DATA 0x10

/*
  Multiboot header

  Multiboot header should be located within the first 8KB of the image and must be
  32-bit aligned. Since our image is very small this will always be under 8KB.
*/
.section .multiboot
.align 4
multiboot_header:
  .long 0x1badb002      # multiboot header magic
  .long 0               # flags bitmap - we don't use any of this for now
  .long 0xe4524ffe      # checksum - when added to magic and flags this must give zero
  .long 0, 0, 0, 0, 0   # the address fields of multiboot header are unused
  .long 1               # video mode - set standard EGA 80x25 text mode
  .long 80              # video mode - 80 columns
  .long 25              # video mode - 25 rows
  .long 0               # video mode depth - should be 0 for a text mode


.text

/*
  Kernel starts execution here.

  The kernel is linked at the linear address 0xff000000. The page tables are located at
  0xffc00000. So that means the kernel size must be less than 12MB.
*/
.global _start
_start:
  /* Memory initialization steps */

  /*
    Multiboot loader will pass information in %ebx, let us store that somewhere before
    initializing memory.
  */
  /* Check whether memory info is available - bit 6 of multiboot_info[0] */
  movl  (%ebx), %eax
  andl  $64, %eax
  jnz   2f
halt:
  hlt
  jmp   halt
2:

  /* %esi points to the start of mmap */
  movl  48(%ebx), %esi
  /* %ecx contains the length of mmap */
  movl  44(%ebx), %ecx
  /* we can support upto 2KB of length */
  cmpl  $2048, %ecx
  jg    halt
  movl  %ecx, mmap_length
  /* %edi points to multiboot_mmap */
  movl  multiboot_mmap, %edi
  /* now we copy the mmap to multiboot_mmap */
  rep movsb


  /* Set up GDT and segment registers */
  physical_addr_of  $gdt, %eax
  physical_addr_of  $gdt_descriptor, %ebx
  movl  %eax, 2(%ebx)

  lgdtl (%ebx)
  movw  $KERNEL_DATA, %ax
  movw  %ax, %ds
  movw  %ax, %es
  movw  %ax, %fs
  movw  %ax, %gs


  /*
    Populate page tables. The kinit and kernel sizes are always a multiple of
	  4KB, the linker script is set up that way. We first identity map the kinit
	  page table entries. And then map kernel pages to 0xff000000.
  */
.macro fill_page_table linear_start, linear_end, physical_start, page_table_addr
	/* edi points to the base of page table */
	movl  \page_table_addr, %edi
  /* ecx holds the index into page table */
  movl  \linear_start, %ecx
  andl  $0x3ff000, %ecx
  shrl  $10, %ecx
  /* and now edi points to the first valid entry in the page table */
  addl  %ecx, %edi

  movl  \linear_end, %ecx
  subl  \linear_start, %ecx
  /* ecx holds the count of pages */
  shrl  $12, %ecx
  /* eax holds the page table entry for the first page */
  setup_paging_entry \physical_start, %eax
  /* clear DF so that edi will get incremented after each stosl */
  cld
1:
  stosl %eax, (%edi)
  /* change eax to point to next page */
  addl  $4096, %eax
  decl  %ecx
  /* repeat for all pages */
  jnz 1b
.endm

  fill_page_table $kinit_start, $kinit_end, $kinit_start, $kinit_page_table
  physical_addr_of  $kernel_page_table, %eax
  physical_addr_of  $kernel_start, %ebx
  fill_page_table $kernel_start, $kernel_end, %ebx, %eax

  /*
    Populate the page directory. We need only two page directory entries. One
	  for kinit and another for kernel.
  */
.macro fill_page_dir page_table_addr, start_addr
  /* eax holds the page directory entry */
	setup_paging_entry \page_table_addr, %eax
  /* ebx points to the base of page directory */
  movl  $kinit_page_dir, %ebx
  /* edi is the index of PDE */
  movl  \start_addr, %edi
  shrl  $22, %edi
  /* copy the paging entry to the correct position in the directory */
  movl  %eax, (%ebx, %edi, 4)
.endm

  fill_page_dir $kinit_page_table, $kinit_start
	physical_addr_of  $kernel_page_table, %ebx
  fill_page_dir %ebx, $kernel_start

  /*
    We also set up a recursive page directory mapping. The idea is to map the
	  last entry in the page directory to point to the start of the page directory
	  itself. Then CPU will consider the page directory as a page table while
	  accessing the last 4MB of linear address space. Each of the page tables will
	  appear as a page of memory in the last 4MB linear address space. And the
	  last 4KB of linear address space will point to the page directory itself.
	  Thus we can manipulate the page directory and page tables without figuring
	  out their physical location. This is a huge convenience after paging is
	  enabled.
  */
  /* eax holds the page directory entry for the page directory address itself */
  setup_paging_entry $kinit_page_dir, %eax
  /* edi points to the page directory */
  movl  $kinit_page_dir, %edi
  /* copy the paging entry to the last entry in the directory */
  movl  %eax, 4092(%edi)

  /* We are done setting up all paging data structures, let us enable paging */
  movl  $kinit_page_dir, %eax
  movl  %eax, %cr3
  movl  %cr0, %eax
  orl   $0x80000000, %eax
  movl  %eax, %cr0

	/* Change all GDT addresses to linear from physical */
	movl  $gdt, gdt_descriptor + 2
	lgdt  gdt_descriptor

  /* Now that paging is enabled, do a long jump to set up %cs correctly */
  ljmp  $KERNEL_CODE, $1f
1:

  /* Set up all segment registers again, this time with linear address in GDT */
  movw    $KERNEL_DATA, %ax
  movw    %ax, %ds
  movw    %ax, %es
  movw    %ax, %fs
  movw    %ax, %gs
  movw    %ax, %ss
  movl    $kernel_stack_end, %esp

  /* Let us switch to C */
  call    kernel_main

  /* We should never reach here because kernel_main doesn't return, but just in case */
  jmp     halt



.bss

/*
  The page directory and table for kernel initialization are located here. Both should
  be aligned to 4KB.
*/
.align 4096
kinit_page_dir:
  .fill 1024, 4, 0

.align 4096
kinit_page_table:
  .fill 1024, 4, 0

/*
  A 4KB kernel stack
*/
.align 4096
kernel_stack_start:
  .fill 1024, 4, 0
kernel_stack_end:

/* Length of memory map as given by multiboot loader */
.global mmap_length
mmap_length:
  .long 0

/*
  Temporarily area to hold mmap provided by multiboot loader.
*/
.global multiboot_mmap
multiboot_mmap:
  .fill 256, 8, 0


/*
  The physical memory bit map is stored at the end of kernel. It can have a maximum size
  of 128KB to represent 4GB of memory. But during PMM initialization we'll shrink it's
  size to match with the actual memory present in the system.
*/
.section .membitmap, "aw", @nobits

.global pmm_bitmap_array
pmm_bitmap_array:
  .fill   128 * 1024, 1, 0
