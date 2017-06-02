/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#include "stdint.h"
#include "debug.h"

static uint8_t kernel_stack[4096];

// This cannot be a function call because stack is not yet setup :)
#define init_stack()                 \
  {                                  \
    uint16_t x;                      \
    __asm__ volatile(                \
      "movw %%ds, %0  ;"             \
      "movw %0, %%ss  ;"             \
      "movl %1, %%esp ;"             \
      : "=q" (x)                     \
      : "i" (&kernel_stack[4096]));  \
  }

int main()
{
  init_stack();

  kdebug("Booting Trantor OS...\n");

  extern void init_memory();

  init_memory();

  for(;;);
}
