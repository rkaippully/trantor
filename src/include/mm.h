/*
  Trantor Operating System

  Copyright (C) 2014 Raghu Kaippully
*/

#ifndef _MM_H
#define _MM_H

#include "ints.h"

static const uint16_t
  KERNEL_CS_SEL = 0x08,
  KERNEL_DS_SEL = 0x10,
  USER_CS_SEL   = 0x1b,
  USER_DS_SEL   = 0x23,
  TSS_SEL       = 0x28;

/* Size of a memory page - physical and virtual */
static const int PAGE_SIZE = 4096;

extern void* kmalloc(uint32_t size);
extern void kfree(void* ptr);

#endif
