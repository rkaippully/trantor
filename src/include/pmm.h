/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#ifndef _PMM_H
#define _PMM_H

/*
  Physical memory management

  PMM handles physical address space. It is responsible for allocating and
  releasing pages of physical memory that can then be used for paging, DMA etc.

  PMM stores information about the availability of physical memory in a bitmap
  located at 0xc8000000. Each set bit in the bitmap indicates a free page.
 */

#include "stdint.h"

static const uint32_t PAGE_SIZE = 4096;

extern uint32_t pmm_bitmap;

// Size of the PMM bitmap in bits
extern uint32_t bitmap_size;

/*
  Allocate a physical page. Returns the base address of the page or zero if
  allocation fails.
*/
extern uint32_t pmm_alloc();

/*
  Frees a previously allocated physical page
*/
extern void pmm_free(uint32_t addr);

typedef uint64_t __attribute__((aligned(8))) GDTEntry;

typedef struct {
  uint16_t limit;
  GDTEntry* base;
} __attribute__((packed)) GDTDescriptor;

extern GDTDescriptor gdt_descriptor;
extern GDTEntry gdt[];

#endif
