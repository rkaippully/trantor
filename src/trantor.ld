/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

/*
  Trantor linker script

  A linker script gives us a lot of control over the generated kernel image, such as
  the load address, output format, layout of sections in the image etc.
*/

OUTPUT_FORMAT("coff-go32")

/* The symbol at which execution starts */
ENTRY(_start)

SECTIONS
{
  /*
    The image is loaded at physical address 0x00100000. The kinit sections are
    located at 0x00100000 in the linear address space. The kernel sections are
    located at 0xf0000000 in the linear address space.
  */
  . = 0x00100000;
  kinit_start = .;

  .kinit.text :
  {
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

  . = 0xf0000000;
  kernel_start = .;

  .kernel.text : AT(kinit_end)
  {
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

    kernel_end = ALIGN(4096);
  }

  /* Stuff we don't want in the kernel */
  /DISCARD/ :
  {
    *(.eh_frame)
    *(.comment)
  }
}
