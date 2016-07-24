/*
  Trantor Operating System

  Copyright (C) 2016 Raghu Kaippully
*/

/*
  Trantor linker script

  A linker script gives us a lot of control over the generated kernel image, such as
  the load address, output format, layout of sections in the image etc.
*/

/* Use 32 bit format */
OUTPUT_FORMAT("elf32-i386")

/* The symbol at which execution starts */
ENTRY(_start)

SECTIONS
{
  /*
    The image is loaded at physical address 0x00100000. The kinit sections are
    located at 0x00100000 in the linear address space. The kernel sections are
    located at 0xff000000 in the linear address space.
  */
  .kinit.text 0x00100000 :
  {
    kinit_start = .;

    /* Put the multiboot section at the begining so that it is within the first 8KB */
    kinit*(.multiboot)

    kinit*(.text)
    kinit*(.text.*)
  }

  /* Data section follows text section */
  .kinit.data :
  {
    kinit*(.data)
    kinit*(.data2)
    kinit*(.rodata)
    kinit*(.rodata.*)
  }

  /* BSS follows the data section */
  .kinit.bss :
  {
    kinit*(.bss)
    kinit*(COMMON)

    kinit_end = ALIGN(4096);
  }

  .kernel.text 0xff000000 : AT(kinit_end)
  {
    kernel_start = .;

    kernel*(.text)
    kernel*(.text.*)
  }

  /* Data section follows text section */
  .kernel.data :
  {
    kernel*(.data)
    kernel*(.data2)
    kernel*(.rodata)
    kernel*(.rodata.*)
  }

  /* BSS follows the data section */
  .kernel.bss :
  {
    kernel*(.bss)
    kernel*(COMMON)

    /* Physical memory bitmap is stored at the end of kernel */
    *(.membitmap)

    kernel_end = ALIGN(4096);
  }

  /* Stuff we don't want in the kernel */
  /DISCARD/ :
  {
    *(.eh_frame)
    *(.comment)
  }
}
