/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  This Source Code Form is "Incompatible With Secondary Licenses", as
  defined by the Mozilla Public License, v. 2.0.
*/

/*
  Trantor kernel linker script
*/

OUTPUT_FORMAT("elf32-i386")
ENTRY(main)

SECTIONS
{
  /* The kernel is loaded at linear address 0xf0000000 and physical address 0x00100000 */
  .text.multiboot 0xf0000000 : AT(0x00100000)
  {
    _kernel_start = .;
    *(.multiboot)
  }

  .text :
  {
    *(.text)
    *(.text.*)
  }

  .data :
  {
    *(.data)
    *(.data2)
    *(.rodata)
    *(.rodata.*)
  }

  .bss :
  {
    *(.bss)
    *(COMMON)

    . = ALIGN(4096);
    _kernel_end = .;
  }

  /* Stuff we don't want in the kernel */
  /DISCARD/ :
  {
    *(.eh_frame)
    *(.comment)
  }
}
