/*
  Trantor Operating System

  Copyright (C) 2014 Raghu Kaippully
*/

#ifndef _MM_H
#define _MM_H

#include "ints.h"

extern uint8_t kinit_start, kinit_end, kernel_start, kernel_end;

extern void* kernel_heap_start;
extern const void* kernel_heap_end;

static const void* const kistart = &kinit_start;
static const void* const kiend = &kinit_end;
static const void* const kstart = &kernel_start;
static const void* const kend = &kernel_end;

/* Size of a memory page - physical and virtual */
static const int PAGE_SIZE = 4096;
static const uint32_t HIGHMEM = 16*1024*1024;

typedef uint32_t phys_page;

/* PMM stack for allocating physical memory */
typedef struct {
  phys_page* base;
  phys_page* top;
} pmm_stack;

extern pmm_stack low_stack, high_stack;

static inline void push(pmm_stack* s, phys_page page)
{
  *(s->top) = page;
  s->top++;
}

static inline phys_page pop(pmm_stack* s)
{
  if (s->top == s->base)
    return 0;
  else {
    phys_page page = *(s->top);
    s->top--;
    return page;
  }
}

extern void memory_map(uint32_t phys_addr, const void* virt_addr, int num_pages);
extern void alloc_virt_page(const void* addr);
extern void free_virt_page(const void* addr);

#endif
