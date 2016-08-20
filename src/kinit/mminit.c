/*
  Trantor Operating System

  Copyright (C) 2016 Raghu Kaippully
*/

#include "ints.h"
#include "kdebug.h"
#include "mm.h"
#include "mminternal.h"

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

/*
  Push `count` number of physical pages to the pmm stacks if they are not used
  by kernel.
*/
static void maybe_push_pages(uint32_t base, int count)
{
  extern uint8_t kernel_postlude_end;
  /* This is the last physical address used by kernel */
  const void* kpend = (void*)&kernel_postlude_end - kstart + kiend;

  for (uint32_t i = 0, addr = base; i < count; i++, addr += PAGE_SIZE) {
    if (addr >= (uint32_t)kistart && addr < (uint32_t)kpend)
      continue;

    kdebug("Push addr=0x%08x\n", addr);
    if (addr < HIGHMEM)
      push(&low_stack, addr);
    else
      push(&high_stack, addr);
  }
}

/*
  Memory Manager initialization

  We look at the memory information given by multiboot loader and allocate two
  stacks that contain the addresses of free pages. The first stack - "low" stack
  - handles memory below 16MB and the second stack - "high" stack - handle
  memory above 16MB.
*/
void mm_init()
{
  /* First check how much memory we have */
  uint32_t low_total = 0, high_total = 0;
  mmap* const mmap_start = &multiboot_mmap;
  mmap* const mmap_end = ((void*)mmap_start) + mmap_length;
  for (mmap* ptr = mmap_start; ptr < mmap_end; ptr = (void*)ptr + ptr->size + 4) {
    if (ptr->type == 1 && ptr->base_high == 0) {
      kdebug("base=0x%08x%08x, length=0x%08x%08x\n", ptr->base_high,
             ptr->base_low, ptr->length_high, ptr->length_low);

      /* We might have more than 4GB RAM, limit it to 4GB in that case */
      uint64_t end = ptr->length_high;
      end = (end << 32) + ptr->length_low + ptr->base_low;
      if (end > 0x100000000ull)
        ptr->length_low = 0x100000000ull - ptr->base_low;

      /* Both base address and length should be page aligned */
      int offset = ptr->base_low % PAGE_SIZE;
      if (offset) {
        ptr->base_low += PAGE_SIZE - offset;
        ptr->length_low -= PAGE_SIZE - offset;
      }
      offset = ptr->length_low % PAGE_SIZE;
      if (offset) {
        ptr->length_low -= offset;
      }

      /*
        We don't use the first page of RAM. This will preserve the real mode IVT
        if if we need it. But more importantly, a return value of 0 from
        alloc_phys_page() is used to indicate an error in allocation.
      */
      if (ptr->base_low == 0) {
        ptr->base_low += PAGE_SIZE;
        ptr->length_low -= PAGE_SIZE;
      }

      /* The region might be straddling 16MB mark */
      if (ptr->base_low >= HIGHMEM) {
        high_total += ptr->length_low;
      } else {
        uint32_t end = ptr->base_low + ptr->length_low;
        if (end <= HIGHMEM)
          low_total += ptr->length_low;
        else {
          low_total += HIGHMEM - ptr->base_low;
          high_total += end - HIGHMEM;
        }
      }
    }
  }

  /* How many pages do we have? */
  uint32_t low_page_count = low_total / PAGE_SIZE;
  uint32_t high_page_count = high_total / PAGE_SIZE;
  kdebug("low_total = 0x%08x, high_total = 0x%08x\n", low_total, high_total);

  /* We need (low_page_count+high_page_count)*4 bytes for the stack. */
  uint32_t pages_for_stack = ceil(low_page_count+high_page_count, 1024);
  kdebug("low_page_count = %d, high_page_count = %d, pages_for_stack = %d\n", low_page_count, high_page_count, pages_for_stack);

  /* First get physical memory for stack */
  int i = 0;
  void* addr = kernel_heap_start;
  for (mmap* ptr = mmap_start; ptr < mmap_end; ptr = (void*)ptr + ptr->size + 4) {
    if (ptr->type == 1 && ptr->base_high == 0 && ptr->length_low >= PAGE_SIZE) {
      /* All initial pages are allocated to the stack */
      if (i < pages_for_stack) {
        int num_pages = min(pages_for_stack-i, ptr->length_low/PAGE_SIZE);
        kdebug("For stack: %d pages at phys=0x%08x linear=0x%08x\n", num_pages, ptr->base_low, addr);
        memory_map(ptr->base_low, addr, num_pages);
        addr += num_pages * PAGE_SIZE;
        i += num_pages;

        /* Set up the stack top pointers if we are done setting up the stack */
        if (i == pages_for_stack) {
          low_stack.base = low_stack.top = kernel_heap_start;
          high_stack.base = high_stack.top = low_stack.top + low_page_count;

          // Kernel heap starts at the next page boundary after the stacks
          kernel_heap_start = high_stack.top + high_page_count;
          uint32_t offset = (uint32_t)kernel_heap_start % PAGE_SIZE;
          if (offset != 0)
            kernel_heap_start = (void*)((uint32_t)kernel_heap_start + PAGE_SIZE - offset);

          /* Push any remaining pages to the stack */
          maybe_push_pages(ptr->base_low + num_pages*PAGE_SIZE, ptr->length_low/PAGE_SIZE - num_pages);
        }
      }
      /* The stack is set up, push these pages to the appropriate stack */
      else {
        maybe_push_pages(ptr->base_low, ptr->length_low/PAGE_SIZE);
      }
    }
  }
}
