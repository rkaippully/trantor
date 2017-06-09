/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#include "stdint.h"
#include "printf.h"
#include "version.h"

uint8_t kernel_stack[4096];

void kmain()
{
  kdebug("Booting Trantor OS...\n");

  extern void init_console(), init_memory();

  init_console();
  cprintf("Welcome to Trantor OS v%d.%d.%d\n\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

  init_memory();

  for(;;);
}

// This cannot be a C function because stack is not setup yet
__asm__(
  ".global _main                        ;"
  "_main:                               ;"
  "movw  %ds, %ax                       ;"
  "movw  %ax, %ss                       ;"
  "movl  $(_kernel_stack + 4096), %esp  ;"
  "call  _kmain                         ;");
