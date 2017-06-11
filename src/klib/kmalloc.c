/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#include "kmalloc.h"
#include "stdint.h"
#include "asm.h"
#include "vmm.h"
#include "printf.h"

/*
  Kernel heap is located at 0xc0c00000-0xc0ffffff. It can't exceed 4MB.

  The alogorithm to allocate and free memory is simple. There are bins of predefined
  sizes - 16bytes to 4k. Each bin is a linked list of bin entries. Bin entries are
  stored in pages of their own. Each bin entry points to the linear address of a page
  that is divided to memory chunks of the bin's size.
*/

static void* const KHEAP_START = (void*)0xc0c00000;

/* A bitmap to indicate which pages are allocated - 1024 bits will do */
static uint32_t km_bmap[32];

/*
  Find an empty page in kernel heap where we can allocate memory.
*/
static void* find_empty_page()
{
  for (int i = 0; i < 32; i++) {
    if (km_bmap[i] != 0xffffffff) {
      uint32_t pos = bsf(~km_bmap[i]) - 1;
      void* p = KHEAP_START + (i*32 + pos) * PAGE_SIZE;
      vmm_alloc((uint32_t)p);
      km_bmap[i] |= 1 << pos;
      return p;
    }
  }

  return 0;
}

typedef struct _bin_entry {
  /* The page where memory for this bin entry is located */
  void* addr;
  /* The next entry in this bin */
  struct _bin_entry* next;
  /* The list of free memory areas in this entry. Forms a linked list. */
  void* free_list;
  /*
    Number of bin-sized allocations made in this entry. When it reaches 0, we can free
    the pages of this bin entry.
  */
  uint32_t alloc_count;
} bin_entry;

/*
  Bin entries themselves require some memory to reside, so I allcoate separate pages
  for them and keep the free ones in this list.
*/
static bin_entry* free_bin_entries = 0;

/*
  Get an available bin entry. If none are available, allocate a new page and add bin
  entries there.
*/
static bin_entry* get_bin_entry()
{
  if (!free_bin_entries) {
    /* Allocate a new page and create entries */
    bin_entry* first = find_empty_page();
    if (!first)
      kpanic("Could not allocate a page.\n");

    bin_entry* p = first;
    for (int i = PAGE_SIZE/sizeof(bin_entry); i > 1; i--, p++)
      p->next = p+1;
    p->next = 0;
    free_bin_entries = first;
  }

  bin_entry* p = free_bin_entries;
  free_bin_entries = p->next;
  return p;
}

/* Bins of size 16bytes to 4k */
#define NUM_BINS 9
static bin_entry* bins[NUM_BINS];

static mutex_t km_mutex;

void* kmalloc(uint32_t size)
{
  int bin_idx = 0;
  while (bin_idx < NUM_BINS && size > 1 << (bin_idx + 4))
    bin_idx++;
  if (bin_idx >= NUM_BINS)
    kpanic("kmalloc(): invalid allocation size %d.\n", size);

  acquire_mutex(&km_mutex);

  /* First check if we have some free space in the bin */
  bin_entry* p;
  for (p = bins[bin_idx]; p; p = p->next)
    if (p->free_list)
      /* we have some free space in this entry */
      break;

  /* No free space, allocate a new page for the bin */
  if (!p) {
    /* First find an empty page. */
    void* page = find_empty_page();
    if (!page)
      kpanic("kmalloc(): could not allocate a page.\n");
    kdebug("kmalloc(): new page for bin %d at 0x%08x\n", bin_idx, page);

    /* Get a bin entry for this page */
    p = get_bin_entry();
    kdebug("kmalloc(): bin entry at 0x%08x\n", p);
    p->addr = page;
    p->next = bins[bin_idx];
    bins[bin_idx] = p;
    p->alloc_count = 0;

    int bin_size = 1 << (bin_idx + 4);
    for (void* next = page; next < page + PAGE_SIZE; next += bin_size) {
      *((void**)next) = p->free_list;
      p->free_list = next;
    }
  }

  /* allocate the space from free_list */
  void* mem = p->free_list;
  p->free_list = *((void**) mem);
  p->alloc_count++;
  kdebug("kmalloc(): returning 0x%08x\n", mem);

  release_mutex(&km_mutex);

  return mem;
}

void kfree(void* block)
{
  /* Find the page that contains this block */
  void* page = (void*)((uint32_t)block & 0xfffff000);

  acquire_mutex(&km_mutex);

  /* Search for this page in all bins */
  for (int bin_idx = 0; bin_idx < NUM_BINS; bin_idx++) {
    for (bin_entry* p = bins[bin_idx]; p; p = p->next) {
      if (p->addr == page) {
        kdebug("kfree(): freeing 0x%08x bin_idx = %d\n", block, bin_idx);
        if (p->alloc_count == 1) {
          /* We are about to free the last entry in this page. Free the entrire page instead. */
          kdebug("kfree(): freeing page 0x%08x\n", page);
          vmm_free((uint32_t)p->addr);
          p->next = free_bin_entries;
          free_bin_entries = p;
        } else {
          *((void**)block) = p->free_list;
          p->free_list = block;
          p->alloc_count--;
        }

        release_mutex(&km_mutex);
        return;
      }
    }
  }

  release_mutex(&km_mutex);
}
