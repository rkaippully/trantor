/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#ifndef _ASM_H
#define _ASM_H

#include "stdint.h"

static inline void sti()
{
  __asm__ volatile("sti");
}

static inline void cli()
{
  __asm__ volatile("cli");
}

static inline void halt()
{
  __asm__ volatile(
    "1:       "
    "hlt     ;"
    "jmp  1b ;");
}

static inline uint8_t inb(uint16_t port)
{
  uint8_t ret;
  __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

static inline void outb(uint16_t port, uint8_t val)
{
  __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline void io_wait()
{
  /* Port 0x80 is unused */
  __asm__ volatile("outb %%al, $0x80" : : "a"(0));
}

/* mutex operations */
typedef uint32_t mutex_t;

static inline void acquire_mutex(mutex_t* m)
{
  __asm__ volatile(
     "lock   btsl $0, %0     ;"
     "jnc    2f              ;"
     "1:                      "
     "pause                  ;"
     "testl  $1, %0          ;"
     "je     1b              ;"
     "lock   btsl $0, %0     ;"
     "jc     1b              ;"
     "2:                      "
     : "+m"(*m));
}

static inline void release_mutex(mutex_t* m)
{
  __asm__ volatile("movl  $0, %0" : "=m"(*m));
}

static inline uint32_t bsf(uint32_t v)
{
  return __builtin_ffs(v);
}

#endif
