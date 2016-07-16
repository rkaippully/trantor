/*
    Trantor Operating System

    Copyright (C) 2014 Raghu Kaippully
*/

/* The start locations of the kernel - physical and virtual */
#define KERNEL_PHYS_START 0x00100000
#define KERNEL_VIRT_START 0xf0000000

/* _kernel_end is defined by the LD script */
#define KERNEL_SIZE (kernel_end - KERNEL_VIRT_START)

/* Convert a virtual address to physical address. This is needed till we enable paging */
#define physical_addr_of(x) ((x) - KERNEL_VIRT_START + KERNEL_PHYS_START)

/* Create a code descriptor with the specified base address, limit, and DPL */
#define gdt_code_descriptor(base, limit, dpl) \
    ( \
        ((limit) & 0xffff) | \
        (((base) & 0xffffff) << 16) | \
        (0x0a << 40) | \
        (((((dpl) & 3) << 5) + 0x90) << 40) | \
        (0x0c << 52) | \
        ((((limit) >> 16) & 0x0f) << 48) | \
        (((base) >> 24) << 56) \
    )

/* Create a data descriptor with the specified base address, limit, and DPL */
#define gdt_data_descriptor(base, limit, dpl) \
    ( \
        ((limit) & 0xffff) | \
        (((base) & 0xffffff) << 16) | \
        (0x02 << 40) | \
        (((((dpl) & 3) << 5) + 0x90) << 40) | \
        (0x0c << 52) | \
        ((((limit) >> 16) & 0x0f) << 48) | \
        (((base) >> 24) << 56) \
    )

#define PRESENT     1
#define READ_WRITE  2

/* Create a page directory/table entry for the specified address and store it in reg */
.macro setup_paging_entry addr, reg
    movl    \addr, \reg
    andl    $0xfffff000, \reg
    addl    $(PRESENT + READ_WRITE), \reg
.endm

/* Segment selectors for intialization */
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
    .long   0x1badb002      # multiboot header magic
    .long   0               # flags bitmap - we don't use any of this for now
    .long   0xe4524ffe      # checksum - when added to magic and flags this must give zero
    .long   0, 0, 0, 0, 0   # the address fields of multiboot header are unused
    .long   1               # video mode - set standard EGA 80x25 text mode
    .long   80              # video mode - 80 columns
    .long   25              # video mode - 25 rows
    .long   0               # video mode depth - should be 0 for a text mode


/* The text section starts here */
.text

/*
    Kernel starts execution here.

    The kernel is linked at the virtual address 0xf0000000. The page tables are located at
    0xffc00000. So that means the kernel size must be less than 252MB, that should be OK.
*/
.global _start
_start:
    /* Memory initialization steps */


    /*
        Multiboot loader will pass information in %ebx, let us store that somewhere before
        initializing memory.
    */
    /* Check whether memory info is available - bit 6 of multiboot_info[0] */
    movl    (%ebx), %eax
    andl    $64, %eax
    jnz     2f
halt:
    hlt
    jmp     halt
2:

    /* %esi points to the start of mmap */
    movl    48(%ebx), %esi
    /* %ecx contains the length of mmap */
    movl    44(%ebx), %ecx
    /* we can support upto 2KB of length */
    cmpl    $2048, %ecx
    jg      halt
    movl    %ecx, physical_addr_of(mmap_length)
    /* %edi points to multiboot_mmap */
    movl    $physical_addr_of(multiboot_mmap), %edi
    /* now we copy the mmap to multiboot_mmap */
    rep movsb


    /* Set up GDT and segment registers */
    lgdt    physical_addr_of(gdt_descr)
    movw    $KERNEL_DATA, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs


    /*
        Populate page table. The kernel size is always a multiple of 4KB, the linker
        script is set up that way. So we take the kernel size and find out the number of
        pages required by it. The kernel is located at 0xf0000000 - the first entry of the
        page table is for that address.
    */
    movl    $KERNEL_SIZE, %ecx
    /* ecx holds the count of kernel pages */
    shrl    $12, %ecx
    /* eax holds the page table entry for the first kernel page */
    setup_paging_entry $KERNEL_PHYS_START, %eax
    /* edi points to the page table */
    movl    $physical_addr_of(kernel_page_table), %edi
    /* clear DF so that edi will get incremented after each stosl */
    cld
1:
    stosl   %eax, (%edi)
    /* change eax to point to next page */
    addl    $4096, %eax
    decl    %ecx
    /* repeat for all pages */
    jnz     1b


    /*
        Populate the page directory. The kernel size should less than 4MB; so we need only
        one page directory entry.
    */
    /* eax holds the page directory entry for the kernel */
    setup_paging_entry $physical_addr_of(kernel_page_table), %eax
    /* edi points to the page directory */
    movl    $physical_addr_of(kernel_page_dir), %edi
    /* copy the paging entry to the correct position in the directory */
    movl    %eax, ((KERNEL_VIRT_START >> 22) * 4)(%edi)


    /*
        When we turn on paging, eip is still in the range of KERNEL_PHYST_START. But the
        page directory/tables are set up only for KERNEL_VIRT_START. So we'll immediately
        get a page fault (and triple fault). So we also need to temporarily identity map
        the kernel's physical address. This page directory entry can be removed after we
        enable paging and do a long jump to KERNEL_VIRT_START.

        The kernel physical address is located in the first 4MB, so we need a page table
        for that. We'll temporarily use kernel_stack_start for the page table and remove
        it as soon as we turn on paging.
    */
    /* eax holds the page directory entry for the kernel */
    setup_paging_entry $physical_addr_of(kernel_stack_start), %eax
    /* edi points to the page directory */
    movl    $physical_addr_of(kernel_page_dir), %edi
    /* copy the paging entry to the correct position in the directory */
    movl    %eax, (%edi)

    movl    $KERNEL_SIZE, %ecx
    /* ecx holds the count of kernel pages */
    shrl    $12, %ecx
    /* eax holds the identity page table entry for the kernel */
    setup_paging_entry $KERNEL_PHYS_START, %eax
    /* edi points to the 1MB entry in the page table */
    movl    $physical_addr_of(kernel_stack_start) + ((KERNEL_PHYS_START >> 12) * 4), %edi
    /* clear DF so that edi will get incremented after each stosl */
    cld
1:
    /* copy the paging entry to the correct position in the page table */
    stosl   %eax, (%edi)
    /* change eax to point to next page */
    addl    $4096, %eax
    decl    %ecx
    /* repeat for all pages */
    jnz     1b


    /*
        We also set up a recursive page directory mapping. The idea is to map the last
        entry in the page directory to point to the start of the page directory itself.
        Then CPU will consider the page directory as a page table while accessing the last
        4MB of virtual memory. Each of the page tables will appear as a page of memory in
        the last 4MB virtual address space. And the last 4KB of virtual address space will
        point to the page directory itself. Thus we can manipulate the page directory and
        page tables without figuring out their physical location. This is a huge
        convinience after paging is enabled.
    */
    /* eax holds the page directory entry for the page directory address itself */
    setup_paging_entry $physical_addr_of(kernel_page_dir), %eax
    /* edi points to the page directory */
    movl    $physical_addr_of(kernel_page_dir), %edi
    /* copy the paging entry to the last entry in the directory */
    movl    %eax, 4092(%edi)


    /*
        We are done setting up all paging data structures, let us enable paging
    */
    movl    $physical_addr_of(kernel_page_dir), %eax
    movl    %eax, %cr3
    movl    %cr0, %eax
    orl     $0x80000000, %eax
    movl    %eax, %cr0


    /* Now that paging is enabled, do a long jump to set up %cs correctly */
    ljmp    $KERNEL_CODE, $1f
1:
    /* Change all GDT addresses to virtual from physical */
    movl    $gdt, gdt_descr + 2
    lgdt    gdt_descr

    /* We no longer need the identity paging for the first 4MB, remove it */
    movl    $0, kernel_page_dir

    /* Set up all segment registers again, this time with virtual address in GDT */
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



.data

/* Length of memory map as given by multiboot loader */
.global mmap_length
mmap_length:
    .long   0

gdt_descr:
    .word   6*8 - 1                 /* GDT limit, there are 6 entries in GDT */
    .long   physical_addr_of(gdt)   /* Base address of GDT */

/*
    We create a GDT with 4 segment descriptors one code and one data for kernel mode and
    user mode each. This is using a flat memory model; base address of the segments are
    set to 0 and limit set to 4GB.
*/
.align 8
gdt:
    .quad   0                       /* NULL descriptor */

    /* Kernel code segment descriptor - base = 0, limit = 4GB, selector = 0x08 */
    .quad   gdt_code_descriptor(0, 4*1024*1024*1024 - 1, 0)
    /* Kernel data segment descriptor - base = 0, limit = 4GB, selector = 0x10 */
    .quad   gdt_data_descriptor(0, 4*1024*1024*1024 - 1, 0)

    /* User code segment descriptor - base = 0, limit = 4GB, selector = 0x1b */
    .quad   gdt_code_descriptor(0, 4*1024*1024*1024 - 1, 3)
    /* User data segment descriptor - base = 0, limit = 4GB, selector = 0x23 */
    .quad   gdt_data_descriptor(0, 4*1024*1024*1024 - 1, 3)

    /* TSS descriptor. This will be filled in later */
    .quad   0



.bss

/*
    The page directory and table for kernel initialization are located here. Both should
    be aligned to 4KB.
*/
.align 4096
kernel_page_dir:
    .fill   1024, 4, 0

.align 4096
kernel_page_table:
    .fill   1024, 4, 0

/*
    A 4KB kernel stack
*/
.align 4096
kernel_stack_start:
    .fill   1024, 4, 0
kernel_stack_end:

/*
    The interrupt vector table. We also temporarily use this area to hold mmap provided by
    multiboot loader.
*/
.align 4096
.global idt
.global multiboot_mmap
idt:
multiboot_mmap:
    .fill   256, 8, 0


/*
    The physical memory bit map is stored at the end of kernel. It can have a maximum size
    of 128KB to represent 4GB of memory. But during PMM initialization we'll shrink it's
    size to match with the actual memory present in the system.
*/
.section .membitmap, "aw", @nobits

.global pmm_bitmap_array
pmm_bitmap_array:
    .fill   128 * 1024, 1, 0
