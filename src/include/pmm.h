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
  located at the end of kernel memory. Each set bit in the bitmap indicates a
  free page.
*/

#include "stdint.h"

#define PAGE_SIZE 4096

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

typedef uint64_t descriptor_t;

typedef struct {
  uint16_t limit;
  descriptor_t* base;
} __attribute__((packed)) tablereg_t;

extern tablereg_t gdtr;
extern descriptor_t gdt[];

extern uint8_t kernel_stack[PAGE_SIZE];

#define KERNEL_STACK_END (kernel_stack + PAGE_SIZE)

#endif
