/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#include "stdint.h"
#include "printf.h"
#include "asm.h"
#include "drivers.h"

void kmain()
{
  kdebug("Booting Trantor OS...\n");

  extern void init_console(), init_cpu(), init_memory(),
    init_interrupts(), init_proc(), init_pit(), init_drivers();

  init_console();

  init_cpu();
  init_memory();
  init_interrupts();
  init_proc();
  init_pit();
  init_drivers();

  halt();
}

// This cannot be a C function because stack is not setup yet
__asm__(
  ".global _main                        ;"
  "_main:                               ;"
  "movw  %ds, %ax                       ;"
  "movw  %ax, %ss                       ;"
  "movl  $(_kernel_stack + 4096), %esp  ;"
  "call  _kmain                         ;");
