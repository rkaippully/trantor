/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

int i = 10;

#include "stdint.h"

static uint8_t kernel_stack[4096];

// This cannot be a function call because stack is not yet setup :)
#define init_stack() \
  {                                                \
    uint16_t x;                                    \
    __asm__ volatile("movw %%ds, %0  \n"           \
                     "movw %0, %%ss   \n"          \
                     "movl %1, %%esp  \n"          \
                     : "=q" (x)                    \
                     : "i" (&kernel_stack[4096])); \
  }

int main()
{
  init_stack();

  unsigned short* ptr = (unsigned short*)0xbffb8000;
  for (int i = 0; i < 80*25 - 1; i++)
    *ptr++ = 0x7145;

  for(;;);
}
