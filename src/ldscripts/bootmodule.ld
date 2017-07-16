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
  Trantor kinit linker script

  The kinit image has two (kinit and kernel) parts.
*/

OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

SECTIONS
{
  .text 0x00100000 :
  {
    *(.text.startup)
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

  .data.mmap :
  {
    *(.data.mmap)
  }

  .bss :
  {
    *(.bss)
    *(COMMON)

    . = ALIGN(4096);
  }

  /* Stuff we don't want */
  /DISCARD/ :
  {
    *(.eh_frame)
    *(.comment)
  }
}
