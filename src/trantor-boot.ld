/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

/*
  Linker script for boot sector and MBR
*/

OUTPUT_FORMAT("binary")

/* The symbol at which execution starts */
ENTRY(start)

SECTIONS
{
  /* MBR and boot sectors use this */
  .text 0x500 :
  {
    *(.text)
  }

  /* Kernel prelude uses this */
  .prelude 0x700 :
  {
    *prelude.o(.prelude.16)
    *prelude.o(.prelude.32)
  }
}
