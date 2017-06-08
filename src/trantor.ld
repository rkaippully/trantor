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
ENTRY(_main)

SECTIONS
{
  /*
    The kernel image is loaded at physical address 0x00100000 and linear address
    0xc0100000.
  */
  . = 0xc0100000;
  _kernel_start = .;

  .text : AT(0x00100000)
  {
    *(.text)
    *(.text.*)
  }

  /* Data section follows text section */
  .data :
  {
    *(.data)
    *(.data2)
    *(.rodata)
    *(.rodata.*)
  }

  /* BSS follows the data section */
  .bss :
  {
    *(.bss)
    *(COMMON)

    . = ALIGN(4096);
    _kernel_end = .;

    /*
      Bitmap for physical memory manager. This must be located at the very end
      of the kernel image in memory.
     */
    _pmm_bitmap = .;
    . += 128*1024;
  }


  /* Stuff we don't want in the kernel */
  /DISCARD/ :
  {
    *(.eh_frame)
    *(.comment)
  }
}
