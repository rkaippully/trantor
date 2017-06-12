/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#include "asm.h"
#include "interrupts.h"

/* tick every 1ms */
static const uint32_t TICK_RATE = 1000;

/* Initialize PIT */
void init_pit()
{
  uint16_t divisor = 1193182 / TICK_RATE;
  outb(0x43, 0x36);
  outb(0x40, divisor & 0xff);
  outb(0x40, divisor >> 8);

  extern void timer_tick();
  irq_funcs[8] = timer_tick;
}
