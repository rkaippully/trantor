/*
  Trantor Operating System

  Copyright (C) 2014 Raghu Kaippully
*/

#include "ints.h"
#include "mutex.h"
#include "mm.h"

/*
  See comments in kernel/mm.c to understand the memory manager
*/

/*
  Multiboot memory map
*/
typedef struct {
  uint32_t size;
  uint32_t base_low, base_high;
  uint32_t length_low, length_high;
  uint32_t type;
} mmap;

/*
  Multiboot loader gives us the mmap structure
*/
extern uint32_t mmap_length;
extern mmap multiboot_mmap;

extern uint8_t* pmm_bitmap;
extern int pmm_bitmap_length;

/*
  Physical Memory Manager initialization

  We look at the memory information given by multiboot loader and allocate a bit map for
  each available page of RAM. The bit map is stored at the end of the kernel.
*/
static void pmm_init()
{
  /* First check how much memory we have */
  uint32_t total = 0;
  mmap* const mmap_start = &multiboot_mmap;
  mmap* const mmap_end = ((void*)mmap_start) + mmap_length;
  for (mmap* ptr = mmap_start; ptr < mmap_end; ptr = (void*)ptr + ptr->size + 4) {
    if (ptr->type == 1 && ptr->base_high == 0) {
      /* We might have more than 4GB RAM, limit it to 4GB in that case */
      uint32_t given_length = ptr->length_low;
      uint32_t max_length = 0xffffffff - ptr->base_low + 1;
      ptr->length_low = max_length < given_length ? max_length : given_length;

      /* Both base address and length should be page aligned */
      int offset = ptr->base_low % PAGE_SIZE;
      ptr->base_low += PAGE_SIZE - offset;
      offset = ptr->length_low % PAGE_SIZE;
      ptr->length_low -= offset;

      total += ptr->length_low;
    }
  }

  /* How many pages do we have? */
  uint32_t page_count = total / PAGE_SIZE;
  /* Number of bytes we need in the PMM bitmap */
  pmm_bitmap_length = page_count / 8;
  if (page_count % 8)
    pmm_bitmap_length++;
  /* Number of page tables needed for PMM bitmap */
  int bitmap_page_count = pmm_bitmap_length / PAGE_SIZE;
  if (pmm_bitmap_length % PAGE_SIZE)
    bitmap_page_count++;

  /* Mark each available page in the bitmap */
  for (mmap* ptr = mmap_start; ptr < mmap_end; ptr = (void*)ptr + ptr->size + 4) {
    if (ptr->type == 1 && ptr->base_high == 0) {
      for (uint32_t i = 0; i < ptr->length_low; i += PAGE_SIZE) {
        uint32_t addr = ptr->base_low + i;
        int page_num = addr / PAGE_SIZE;
        /* The bit position for the page at addr */
        uint8_t bit_to_set = 1 << (page_num % 8);
        /* Set the bit to indicate that the page is available */
        pmm_bitmap[page_num / 8] |= bit_to_set;
      }
    }
  }

  /*
    Mark the kernel physical memory as used.
  */
  const void* kphys_start = &kinit_end;
  const void* kphys_end = &kernel_end - &kernel_start + kphys_start;
  for (const void* addr = kphys_start;
       addr < kphys_end;
       addr += PAGE_SIZE) {
    int page_num = (uint32_t)addr / PAGE_SIZE;
    /* The bit position for the page at addr */
    uint8_t bit_to_clear = (1 << (page_num % 8));
    /* Clear the bit to indicate that the page is not available */
    pmm_bitmap[page_num / 8] &= ~bit_to_clear;
  }

  /*
    We also mark the first page of RAM unusable. This will preserve the real mode IVT
    if we need it. But more importantly, a return value of 0 from alloc_phys_page() is
    used to indicate an error in allocation.
  */
  pmm_bitmap[0] &= ~1;
}

static void vmm_init()
{
  /*
    We have already set up page directory and page tables in boot.S, but that covered
    even the unused parts of PMM bitmap. Let us free that here.
  */
  void* addr = pmm_bitmap + pmm_bitmap_length;
  /* Align addr to the next page boundary */
  addr += PAGE_SIZE - ((uint32_t)addr % PAGE_SIZE);
  /* Now free all unused pages in [addr, KERNEL_END) */
  while (addr < kend) {
    free_virt_page(addr);
    addr += PAGE_SIZE;
  }
}

/*
  Entry point for memory manager
*/
void mm_init()
{
  pmm_init();
  vmm_init();
}
