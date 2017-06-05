/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#include "stdint.h"
#include "debug.h"
#include "pmm.h"

static const uint32_t PAGE_SIZE = 4096;

typedef struct {
  uint64_t base;
  uint64_t length;
  uint32_t type;
} __attribute__((packed)) mmap_entry;

static uint32_t* mmap_entry_count = (uint32_t*)0xbff00500;  // Physical address 0x500
static mmap_entry* mmap = (mmap_entry*)0xbff00504;          // Physical address 0x504

static void print_mmap(uint32_t count)
{
  for (int i = 0; i < count; i++) {
    kdebug("BIOS memory: base=0x%08x%08x, length=0x%08x%08x, type=%d\n",
           (uint32_t)(mmap[i].base >> 32), (uint32_t)mmap[i].base,
           (uint32_t)(mmap[i].length >> 32), (uint32_t)mmap[i].length,
           mmap[i].type);
  }
}

static uint32_t page_align_up(uint32_t addr)
{
  return ((addr + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
}

static uint32_t page_align_down(uint32_t addr)
{
  return (addr / PAGE_SIZE) * PAGE_SIZE;
}

static void sanitize_mmap(uint32_t count)
{
  // Adjust base and length to align them to page boundary
  for (int i = 0; i < count; i++) {
    uint64_t start = mmap[i].base;
    uint64_t end = mmap[i].base + mmap[i].length;
    if (mmap[i].type == 1) {
      mmap[i].base = page_align_up(start);
      mmap[i].length = page_align_down(end) - mmap[i].base;
    } else {
      mmap[i].base = page_align_down(start);
      mmap[i].length = page_align_up(end) - mmap[i].base;
    }
  }

  // Sort the mmap entries in the increasing order of base address, and then
  // length.
  for (int i = 1; i < count; i++) {
    for (int j = i; j > 0; j--) {
      if (mmap[j - 1].base > mmap[j].base ||
          (mmap[j - 1].base == mmap[j].base && mmap[j - 1].length > mmap[j].length)) {
        mmap_entry tmp = mmap[j - 1];
        mmap[j - 1] = mmap[j];
        mmap[j] = tmp;
      } else
        break;
    }
  }
}

static uint32_t filter_long_memory(uint32_t count)
{
  // Mark all memory above 4GB as unusable
  for (int i = 0; i < count; i++) {
    if (mmap[i].base > 0xffffffff) {
      count = i - 1;
      break;
    }

    uint64_t end = mmap[i].base + mmap[i].length;
    if (end > 0x100000000)
      mmap[i].length = 0x100000000 - mmap[i].base;
  }
  return count;
}

static uint32_t merge_mmap_entries(uint32_t count)
{
  uint32_t total_mem = 0;
  for (int i = 0; i < count; i++) {
    if (mmap[i].type != 1)
      continue;

    uint32_t start = mmap[i].base,
      end = mmap[i].base + mmap[i].length;
    // The first page is reserved for IVT, BDA etc
    if (start == 0)
      start = PAGE_SIZE;

    // There could be overlapping regions of memory in the mmap, check the next
    // mmap entries
    for (int j = i + 1; j < count; j++) {
      uint32_t n_start = mmap[j].base,
        n_end = mmap[j].base + mmap[j].length;

      // Case 1: There is no overlap
      if (n_start >= end)
        break;
      // Case 2: mmap[j] is completely within mmap[i]
      else if (n_end <= end) {
        if (mmap[j].type == 1) {
          // Ignore mmap[j]
          mmap[j].type = 0xff;
        } else {
          // There is a hole in mmap[i]
          //
          // If j does not end at i's end, we need a new usable mementry
          // from j's end to i's end
          if (n_end < end) {
            mmap[j].base = n_end;
            mmap[j].length = end - n_end;
            mmap[j].type = 1;
          }
          end = n_start;
        }
      }
      // Case 3: Part of mmap[j] is within mmap[i]
      else {
        if (mmap[j].type == 1) {
          mmap[j].length -= end - n_start;
          mmap[j].base = end;
        }
        else {
          // There is a hole in mmap[i]
          end = n_start;
        }
      }
    }

    total_mem += end - start;
    mmap[i].base = start;
    mmap[i].length = end - start;
    kdebug("Usable memory: base=0x%08x, length=0x%08x, type=%d\n",
           (uint32_t)mmap[i].base, (uint32_t)mmap[i].length, mmap[i].type);
  }
  kdebug("Usable memory: 0x%08x bytes\n", total_mem);

  return total_mem;
}

// Convert a linear address to physical address
static uint32_t linear_to_physical(uint32_t addr)
{
  return addr + 0x40100000;
}

static void create_pmm_stack(uint32_t count, uint32_t total_mem)
{
  // Defined in the linker script
  extern uint8_t kernel_start, kernel_end;

  /*
    We create a bitmap of the free pages. Each set bit in the bitmap represents
    one free page of physical address space.
  */
  uint32_t kernel_size = &kernel_end - &kernel_start;
  uint32_t free_mem_pages = (total_mem - kernel_size) / PAGE_SIZE;
  kdebug("kernel size = %d bytes, free mem page count = %d\n", kernel_size, free_mem_pages);

  /* One page of bitmap represents 32768 pages. */
  uint32_t bitmap_pages = free_mem_pages / 32769 + 1;
  uint32_t available_mem_pages = free_mem_pages - bitmap_pages;
  kdebug("bitmap page count = %d, available page count = %d\n",
         bitmap_pages, available_mem_pages);

  uint32_t max_mem = 0;
  // Mark all type 1 memory as available
  uint8_t* p = &pmm_bitmap;
  for (int i = 0; i < count; i++) {
    if (mmap[i].type != 1)
      continue;

    max_mem = mmap[i].base + mmap[i].length;
    for (uint32_t page = mmap[i].base; page < mmap[i].base + mmap[i].length; page += PAGE_SIZE) {
      uint32_t n = page / PAGE_SIZE;
      p[n/8] |= 1 << (n % 8);
    }
  }

  // Mark IVT, BDA as unavailable
  p[0] &= 0xfe;

  // Mark kernel as unavailable
  for (uint32_t page = (uint32_t)&kernel_start; page < (uint32_t)&kernel_end; page += PAGE_SIZE) {
    uint32_t n = linear_to_physical(page) / PAGE_SIZE;
    p[n/8] &= ~((uint8_t)1 << (n % 8));
  }
  // Mark bitmap pages as unavailable
  for (uint32_t page = (uint32_t)p; page < (uint32_t)p + bitmap_pages*PAGE_SIZE; page += PAGE_SIZE) {
    uint32_t n = linear_to_physical(page) / PAGE_SIZE;
    p[n/8] &= ~((uint8_t)1 << (n % 8));
  }

  uint32_t bitmap_size = max_mem / PAGE_SIZE;  // Size in bits
  kdebug("PMM bitmap:\n");
  for (int i = 0; i*8 < bitmap_size; i++) {
    kdebug("%02x ", p[i]);
    if ((i % 16) == 15)
      kdebug("\n");
  }
  kdebug("\n");
}

void init_memory()
{
  uint32_t count = *mmap_entry_count;

  print_mmap(count);

  sanitize_mmap(count);
  count = filter_long_memory(count);
  uint32_t total_mem = merge_mmap_entries(count);
  create_pmm_stack(count, total_mem);
}
