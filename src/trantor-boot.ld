/*
  Trantor Operating System

  Copyright (C) 2016 Raghu Kaippully
*/

/*
  Linker script for boot sector and MBR
*/

OUTPUT_FORMAT("binary")

/* The symbol at which execution starts */
ENTRY(start)

SECTIONS
{
  .text 0x500 :
  {
    *(.text)
  }
}
