/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#include "asm.h"
#include "pmm.h"

// Size of the PMM bitmap in bits
uint32_t bitmap_size;

// Index at which we start searching for free mem
static uint32_t bitmap_idx = 1;

uint32_t pmm_alloc()
{
  /* Scan for the lowest set bit */
  uint32_t idx = bitmap_idx;
  uint32_t* ptr = &pmm_bitmap + idx;
  do {
    uint32_t pos = bsf(*ptr);
    if (pos != 0) {
      *ptr &= ~(1 << (pos - 1));
      bitmap_idx = idx;
      return (idx*32 + pos - 1) * PAGE_SIZE;
    } else {
      idx++;
      ptr++;
      if (idx*32 >= bitmap_size) {
        idx = 0;
        ptr = &pmm_bitmap;
      }
    }
  } while (idx != bitmap_idx);

  // We ran out of memory
  return 0;
}

void pmm_free(uint32_t addr)
{
  uint32_t idx = addr / 32;
  uint32_t pos = addr % 32;
  uint32_t* ptr = &pmm_bitmap;
  ptr[idx] |= 1 << pos;
}
