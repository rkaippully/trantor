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
ENTRY(main)

SECTIONS
{
  /* The kinit is loaded at physical/linear address 0x00100000 */
  .text.multiboot 0x00100000 : AT(0x00100000)
  {
    _kinit_start = .;
    *(.multiboot)
  }

  .text :
  {
    *(EXCLUDE_FILE(*kernel*) .text)
    *(EXCLUDE_FILE(*kernel*) .text.*)
  }

  .data :
  {
    *(EXCLUDE_FILE(*kernel*) .data)
    *(EXCLUDE_FILE(*kernel*) .data2)
    *(EXCLUDE_FILE(*kernel*) .rodata)
    *(EXCLUDE_FILE(*kernel*) .rodata.*)
  }

  .bss :
  {
    *(EXCLUDE_FILE(*kernel*) .bss)
    *(EXCLUDE_FILE(*kernel*) COMMON)

    . = ALIGN(4096);
    _kinit_end = .;
  }

  /* The kernel is loaded at linear address 0xf0000000 */
  .text.kernel 0xf0000000 : AT(ADDR(.bss) + SIZEOF(.bss))
  {
    _kernel_start = .;
    *kernel*(.text)
    *kernel*(.text.*)
  }

  .data.kernel :
  {
    *kernel*(.data)
    *kernel*(.data2)
    *kernel*(.rodata)
    *kernel*(.rodata.*)
  }

  .bss.kernel :
  {
    *kernel*(.bss)
    *kernel*(COMMON)

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
