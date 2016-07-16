/*
    Trantor Operating System

    Copyright (C) 2014 Raghu Kaippully
*/

#ifndef _ASM_H
#define _ASM_H

#include "ints.h"

static inline uint8_t in_byte(uint16_t port)
{
    uint8_t val;
    __asm__ volatile("inb %1, %0" : "=a"(val) : "d"(port));
    return val;
}

static inline void out_byte(uint16_t port, uint8_t val)
{
    __asm__ volatile("outb %1, %0" :: "d"(port), "a"(val));
}

/*
    An ISR will save registers in this order on the stack
*/
typedef struct __attribute__((packed)) {
    uint32_t  gs, fs, es, ds, edi, esi, ebp, esp, ebx, edx, ecx, eax;
} regs_t;

#endif
