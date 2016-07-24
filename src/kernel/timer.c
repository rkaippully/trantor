/*
  Trantor Operating System

  Copyright (C) 2014 Raghu Kaippully
*/

#include "ints.h"
#include "asm.h"

/*
  Time management for the kernel

  We use 8253/8254 PIT to generate a tick approximately every millisecond.
*/

/*
  Variables that track elapsed time since boot
*/
static uint64_t milli_secs_elapsed;
static uint32_t micro_secs_elapsed, nano_secs_elapsed;

/* this is set to true if realtime is stale and need to be reloaded from RTC */
static bool rtc_reload = false;

void timer_init()
{
  /*
    The PIT has a frequency of 1193182 Hz. So we program it to send an IRQ every 1194
    pulses to get a tick approximately every millisecond. Thus the elapsed time
    between two IRQ ticks is 1194 * 1000 / 1193182 = 1.00068556 ms. We track the
    number of milliseconds, microseconds, and nanoseconds elapsed in two variables -
    see timer_isr.
  */
  const uint16_t tick_count = 1194;

  /*
    Send a byte to control word register

    bit 0 = 0 (numbers are in binary format)
    bit 1-3 = 011 (mode 3: square wave generator)
    bit 4-5 = 11 (load LSB first then MSB)
    bit 6-7 = 00 (load channel 0)
  */
  out_byte(0x43, 0x36);

  /* Write LSB first and then MSB */
  out_byte(0x40, tick_count & 0xff);
  out_byte(0x40, tick_count >> 8);
}

/*
  ISR invoked by IRQ0. Each tick is 1.000
*/
void timer_isr()
{
  /* Increment time variables */
  const uint32_t nanos_increment = 686;
  const uint64_t millis_increment = 1;
  nano_secs_elapsed += nanos_increment;
  if (nano_secs_elapsed >= 1000) {
    nano_secs_elapsed -= 1000;
    micro_secs_elapsed++;
  }
  if (micro_secs_elapsed >= 1000) {
    micro_secs_elapsed -= 1000;
    milli_secs_elapsed++;
  }
  milli_secs_elapsed += millis_increment;

  /*
    Update real time from RTC every 500 ticks. We don't actually update the time to
    keep this ISR fast. We instead set a flag here and the next call to get realtime
    clock will get the value from RTC.
  */
  rtc_reload = true;
}
