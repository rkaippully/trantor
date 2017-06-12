/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#include "asm.h"
#include "interrupts.h"
#include "printf.h"

void timer_tick()
{
  kdebug("timer tick\n");
  outb(0x20, 0x20);
}
