/*
    Trantor Operating System
    Copyright (C) 2014 Raghu Kaippully

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
