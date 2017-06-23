/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#include "asm.h"
#include "interrupts.h"
#include "printf.h"
#include "process.h"

void timer_tick()
{
  proc_schedule();

  outb(0x20, 0x20);
}
