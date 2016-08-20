/*
  Trantor Operating System

  Copyright (C) 2014 Raghu Kaippully
*/

/* Stuff internal to memory manager */
#ifndef _MMINTERNAL_H
#define _MMINTERNAL_H

#include "ints.h"

extern uint8_t kinit_start, kinit_end, kernel_start, kernel_end;

extern void* kernel_heap_start;
extern const void* kernel_heap_end;

static const void* const kistart = &kinit_start;
static const void* const kiend = &kinit_end;
static const void* const kstart = &kernel_start;
static const void* const kend = &kernel_end;

static const uint32_t HIGHMEM = 16*1024*1024;

typedef uint32_t phys_page;

/* PMM stack for allocating physical memory */
typedef struct {
  phys_page* base;
  phys_page* top;
} pmm_stack;

extern pmm_stack low_stack, high_stack;

extern uint64_t gdt[6];

/*
  Create a GDT descriptor with the specified base address, limit, DPL, and type
*/
#define make_gdt_descriptor(base, limit, dpl, type) \
  (                                                 \
   ((limit) & 0xffff) |                             \
   (((base) & 0xffffff) << 16) |                    \
   ((uint64_t)type << 40) |                         \
   ((((((uint64_t)dpl) & 3) << 5) + 0x80) << 40) |  \
   ((uint64_t)0x0c << 52) |                         \
   (((((uint64_t)limit) >> 16) & 0x0f) << 48) |     \
   ((((uint64_t)base) >> 24) << 56)                 \
    )

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
