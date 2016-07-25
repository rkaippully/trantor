/*
  Trantor Operating System

  Copyright (C) 2014 Raghu Kaippully
*/

#ifndef _MM_H
#define _MM_H

#include "ints.h"

extern uint8_t kinit_end, kernel_start, kernel_end;

static const void* const kstart = &kernel_start;
static const void* const kend = &kernel_end;

/* Size of a memory page - physical and virtual */
static const int PAGE_SIZE = 4096;

extern void memory_map(uint32_t phys_addr, const void* virt_addr, int num_pages);
extern void alloc_virt_page(const void* addr);
extern void free_virt_page(const void* addr);

#endif
