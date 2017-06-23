/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#ifndef _ASM_H
#define _ASM_H

#include "stdint.h"

/* Well known segment selectors */
#define KERNEL_CS    0x0008
#define KERNEL_DS    0x0010
#define USER_CS      0x001b
#define USER_DS      0x0023
#define TSS_SELECTOR 0x0028

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

static inline uint32_t bsf(uint32_t v)
{
  return __builtin_ffs(v);
}

#endif
