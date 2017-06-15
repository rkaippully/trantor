/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#include "asm.h"
#include "interrupts.h"
#include "printf.h"

void timer_tick()
{
  /*static uint32_t count = 0;
    cprintf("timer tick %d\r", count++);*/
  outb(0x20, 0x20);
}
