/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#include "stdint.h"
#include "printf.h"
#include "asm.h"
#include "pmm.h"
#include "syscap.h"

// Defined in the linker script
extern uint8_t kernel_start, kernel_end;

typedef struct {
  uint64_t base;
  uint64_t length;
  uint32_t type;
} __attribute__((packed)) mmap_entry;

static mmap_entry* mmap;

static uint32_t bitmap_pages;

static void print_mmap(uint32_t count)
{
  for (int i = 0; i < count; i++) {
    kdebug("BIOS memory: base=0x%08x%08x, length=0x%08x%08x, type=%d\n",
           (uint32_t)(mmap[i].base >> 32), (uint32_t)mmap[i].base,
           (uint32_t)(mmap[i].length >> 32), (uint32_t)mmap[i].length,
           mmap[i].type);
  }
}

static uint64_t page_align_up(uint64_t addr)
{
  return ((addr + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
}

static uint64_t page_align_down(uint64_t addr)
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
  uint32_t total_mem = 0, max_mem = 0;
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
          // If j does not end at i's end, we need a new usable mmap entry
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
    max_mem = end;
    mmap[i].base = start;
    mmap[i].length = end - start;
    kdebug("Usable memory: base=0x%08x, length=0x%08x, type=%d\n",
           (uint32_t)mmap[i].base, (uint32_t)mmap[i].length, mmap[i].type);
  }

  kdebug("Usable memory: 0x%08x bytes, max memory: 0x%08x\n", total_mem, max_mem);
  cprintf("Usable memory: %d KB\n", total_mem/1024);

  return max_mem;
}

// Convert a linear address to physical address
static uint32_t linear_to_physical(uint32_t addr)
{
  return addr + 0x40000000;
}

// Convert a physical address to linear address
static uint32_t physical_to_linear(uint32_t addr)
{
  return addr - 0x40000000;
}

static void create_pmm_stack(uint32_t count, uint32_t max_mem)
{
  uint32_t kernel_size = &kernel_end - &kernel_start;
  kdebug("Kernel size = %d bytes\n", kernel_size);

  /*
    We create a bitmap of the free pages. Each set bit in the bitmap represents
    one free page of physical address space.
  */
  bitmap_size = max_mem / PAGE_SIZE;  // Size in bits

  /* One page of bitmap represents 32768 pages. */
  bitmap_pages = bitmap_size / 32768 + 1;
  kdebug("bitmap page count = %d\n", bitmap_pages);

  // Mark all type 1 memory as available
  uint32_t* p = &pmm_bitmap;
  for (int i = 0; i < count; i++) {
    if (mmap[i].type != 1)
      continue;

    for (uint32_t page = mmap[i].base; page < mmap[i].base + mmap[i].length; page += PAGE_SIZE) {
      uint32_t n = page / PAGE_SIZE;
      p[n/32] |= 1 << (n % 32);
    }
  }

  // Mark IVT, BDA as unavailable
  p[0] &= 0xfffffffe;

  // Mark kernel as unavailable
  for (uint32_t page = (uint32_t)&kernel_start; page < (uint32_t)&kernel_end; page += PAGE_SIZE) {
    uint32_t n = linear_to_physical(page) / PAGE_SIZE;
    p[n/32] &= ~(1 << (n % 32));
  }
  // Mark bitmap pages as unavailable
  for (uint32_t page = (uint32_t)p; page < (uint32_t)p + bitmap_pages*PAGE_SIZE; page += PAGE_SIZE) {
    uint32_t n = linear_to_physical(page) / PAGE_SIZE;
    p[n/32] &= ~(1 << (n % 32));
  }

  kdebug("PMM bitmap:\n");
  for (int i = 0; i*32 < bitmap_size; i++) {
    if ((i % 8) == 0)
      kdebug("%05x: ", i);
    kdebug("%02x ", (uint8_t)p[i]);
    kdebug("%02x ", (uint8_t)(p[i] >> 8));
    kdebug("%02x ", (uint8_t)(p[i] >> 16));
    kdebug("%02x ", (uint8_t)(p[i] >> 24));
    if ((i % 8) == 7)
      kdebug("\n");
  }
  kdebug("\n");
}

// Initialize physical memory manager
static void init_pmm()
{
  cprintf("Detecting memory...\n");

  uint32_t* mmap_entry_count = (uint32_t*)physical_to_linear(0x500);
  mmap = (mmap_entry*)physical_to_linear(0x504);

  uint32_t count = *mmap_entry_count;

  print_mmap(count);

  sanitize_mmap(count);
  count = filter_long_memory(count);
  uint32_t max_mem = merge_mmap_entries(count);
  create_pmm_stack(count, max_mem);
}

static void init_paging()
{
  uint32_t kernel_size = &kernel_end - &kernel_start;
  uint32_t kernel_page_count = (kernel_size + PAGE_SIZE - 1)/PAGE_SIZE + bitmap_pages;

  // Kernel is loaded at 1 MB and should not exceed 4 MB
  if (kernel_page_count > 768)
    kpanic("Kernel is too large, needs %d pages.\n");

  uint32_t page_dir_addr = pmm_alloc();
  if (page_dir_addr == 0)
    kpanic("Could not allocate a physical page for page directory.\n");

  uint32_t page_tbl_addr = pmm_alloc();
  if (page_tbl_addr == 0)
    kpanic("Could not allocate a physical page for page table.\n");

  // Enable global pages
  if (syscap.pge) {
    uint32_t tmp = 0;
    __asm__ volatile(
      "movl  %%cr4, %0  ;"
      "btsl  $7, %0     ;"
      "movl  %0, %%cr4  ;"
      : "+r"(tmp));
  }

  uint32_t* pd_ptr = (uint32_t*)physical_to_linear(page_dir_addr);
  uint32_t* pt_ptr = (uint32_t*)physical_to_linear(page_tbl_addr);

  // Clear out the entire page directory and table
  for (int i = 0; i < 1024; i++) {
    pd_ptr[i] = 0;
    pt_ptr[i] = 0;
  }

  // Lower physical memory of 1 MB + kernel is mapped to 0x00000000 and 0xc0000000
  pd_ptr[0] = pd_ptr[0x300] = page_tbl_addr | 0x103;
  for (int i = 0; i < 256 + kernel_page_count; i++)
    pt_ptr[i] = (i * PAGE_SIZE) | 0x103;

  // Create a self mapping for the PD
  pd_ptr[0x3ff] = page_dir_addr | 0x103;

  // Turn on paging
  uint32_t tmp1;
  __asm__ volatile(
    "movl  %1, %%cr3  ;"
    "movl  %%cr0, %0  ;"
    "btsl  $31, %0    ;"
    "movl  %0, %%cr0  ;"
    : "=r"(tmp1)
    : "r"(page_dir_addr));

  // Load new segments
  uint16_t tmp2;
  __asm__ volatile(
    "lgdtl  %1          ;"
    "ljmp   %2, $1f     ;"
    "1:                  "
    "movw   %3, %0      ;"
    "movw   %0, %%ds    ;"
    "movw   %0, %%es    ;"
    "movw   %0, %%fs    ;"
    "movw   %0, %%gs    ;"
    "movw   %0, %%ss    ;"
    : "=r"(tmp2)
    : "m"(gdt_descriptor), "i"(KERNEL_CS), "i"(KERNEL_DS));

  // Remove the identity mapping and reload cr3
  pd_ptr[0] = 0;
  __asm__ volatile("movl  %0, %%cr3" : : "r"(page_dir_addr));
}

// Initialize virtual memory manager
static void init_vmm()
{
  init_paging();
}

void init_memory()
{
  init_pmm();
  init_vmm();
}
