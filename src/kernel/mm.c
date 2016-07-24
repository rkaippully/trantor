/*
  Trantor Operating System

  Copyright (C) 2014 Raghu Kaippully
*/

#include "ints.h"
#include "mutex.h"
#include "mm.h"

/*
  The memory manager

  Memory manager is divided to two parts - physical and virtual. Physical memory manager
  handles 4KB sized pages of physical memory and allocates/deallocates them as requested
  by the virtual memory manager. The virtual memory manager makes use of MMU and
  allocates/deallocates virtual 4KB pages in the virtual address space. It knows about
  the virtual memory layout and can handle things like allocate-on-demand, copy-on-write
  etc.
*/

/*
  Virtual Memory Layout

  0x00000000 - 0xefffffff - user space (most of this will not be used)
  0xf0000000 - 0xffbfffff - kernel (most of this will not be used)
  0xffc00000 - 0xffffffff - page tables and directory
*/

static const void* const KERNEL_START = (void*)0xf0000000;
static const void* KERNEL_END;
static const void* const KERNEL_PHYS_START = (void*)0x00100000;
static const void* KERNEL_PHYS_END;

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
  We have a chicken and egg problem. We need to map the PMM bitmap to virtual
  address space before we can do page mappings. And for that mapping we need to
  allocate physical pages and that needs the PMM bitmap already populated.

  So we have preallocated a 128KB pmm_bitmap in boot.S to cover the entire 32-bit
  address space. Here we check how much memory is available and populate the bit map
  accordingly. After mapping is completed, we release the unused portion of the
  bitmap for future allocations.

  A 0 bit in the bitmap indicates that the page is in use and a 1 bit indicates that it
  is available for allocation.
*/
extern uint8_t pmm_bitmap_array;
static uint8_t* const pmm_bitmap = &pmm_bitmap_array;

/*
  This is the length of PMM bitmap specified in bytes
*/
static int pmm_bitmap_length;

uint32_t kernel_page_table[1024] __attribute__((aligned(4096)));

/* Create a GDT descriptor with the specified base address, limit, DPL, and type */
#define make_gdt_descriptor(base, limit, dpl, type)  \
( \
 ((limit) & 0xffff) |                             \
 (((base) & 0xffffff) << 16) |                    \
 ((uint64_t)type << 40) |                         \
 ((((((uint64_t)dpl) & 3) << 5) + 0x90) << 40) |  \
 ((uint64_t)0x0c << 52) |                         \
 (((((uint64_t)limit) >> 16) & 0x0f) << 48) |     \
 ((((uint64_t)base) >> 24) << 56)                 \
)

uint64_t gdt[6] __attribute__((aligned(8))) = {
  /* NULL decriptor */
  0,
  /* Kernel code segment descriptor - base = 0, limit = 4GB, selector = 0x08 */
  make_gdt_descriptor(0, 0xffffffff, 0, 0x0a),
  /* Kernel data segment descriptor - base = 0, limit = 4GB, selector = 0x10 */
  make_gdt_descriptor(0, 0xffffffff, 0, 0x02),
  /* User code segment descriptor - base = 0, limit = 4GB, selector = 0x1b */
  make_gdt_descriptor(0, 0xffffffff, 3, 0x0a),
  /* User data segment descriptor - base = 0, limit = 4GB, selector = 0x23 */
  make_gdt_descriptor(0, 0xffffffff, 3, 0x02),
  /* TSS descriptor. This will be filled in later */
  0
};

/* LD script sets the location of these addresses. */
extern const uint8_t kinit_start, kinit_end, kernel_start, kernel_end;

struct {
  uint16_t limit;
  void* gdt_addr;
} __attribute__((packed)) gdt_descriptor = {
  6*8 - 1, /* GDT limit, there are 6 entries*/
  gdt
};

/*
  Physical Memory Manager initialization

  We look at the memory information given by multiboot loader and allocate a bit map for
  each available page of RAM. The bit map is stored at physical address 0x00000500 and
  virtual address 0xf8000000.
*/
static void pmm_init()
{
  KERNEL_END = &kernel_end;
  KERNEL_PHYS_END = (void*)((uint32_t)KERNEL_END - 0xf0000000 + 0x00100000);

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
  for (const void* addr = KERNEL_PHYS_START;
       addr < KERNEL_PHYS_END;
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

/*
  Allocates a physical page and returns it's base address. Returns 0 if allocation
  fails.
*/
static uint32_t alloc_phys_page()
{
  /*
    Search through the PMM bitmap and find the next available free page. We store the
    last location where we found an empty page for efficiency reasons.
  */
  static int last_allocated_idx = 0;

  uint32_t idx = last_allocated_idx;
  while (!pmm_bitmap[idx]) {
    if (++idx >= pmm_bitmap_length)
      idx = 0;
    /* Did we run out of memory? */
    if (idx == last_allocated_idx)
      return 0;
  }
  last_allocated_idx = idx;

  /* Memory available at pos, find the first set bit and use that one */
  uint32_t bit_idx;
  __asm__ volatile("bsfl %1, %0" : "=q"(bit_idx) : "m"(pmm_bitmap[idx]));
  pmm_bitmap[idx] &= ~(1 << bit_idx);
  return (idx * 8 + bit_idx) * PAGE_SIZE;
}

/*
  Free an allocated page at address addr. We don't do any checks
*/
static void free_phys_page(uint32_t addr)
{
  uint32_t page = addr / PAGE_SIZE;
  pmm_bitmap[page / 8] |= 1 << (page % 8);
}




/* Virtual memory management functions */

/*
  Page directories and page tables

  Representing these as a union makes it easy to access the individual fields and the
  entire field as a 32-bit int.
*/
struct paging_entry {
  int present:1;
  int writable:1;
  int user_mode:1;
  int unused:9;
  int addr:20;
} __attribute__((__packed__));

/*
  An entry in a page directory/table.
*/
typedef union {
  struct paging_entry fields;
  uint32_t word;
} paging_t;

/*
  We set up a recursive page directory mapping in boot.S by mapping the last page
  directory entry to map to the base of page directory itself. It is easy to see that
  this will make the page directory available at virtual address 0xfffff000 and all the
  page tables available starting at 0xffc00000.
*/
static paging_t* const page_dir = (paging_t*)0xfffff000;
static paging_t* const page_tables = (paging_t*)0xffc00000;

/*
  Allocate a new physical page in the specified paging entry
*/
static inline void new_paging_entry(paging_t* entry, uint32_t phys_addr, bool is_kernel)
{
  /* TODO: handle out of memory */
  entry->fields.present = 1;
  entry->fields.writable = 1;
  entry->fields.user_mode = !is_kernel;
  entry->fields.addr = phys_addr >> 12;
}

/*
  Issue an invlpg instruction to invalidate TLBs
*/
static inline void invlpg(const void* const addr)
{
  __asm__ volatile("invlpg %0" :: "m"(*(uint8_t*)addr));
}

/* A mutex used in allocation and deallocation */
static mutex_t vmm_mutex;

/*
  Memory mapping

  This function is used by device drivers to map physical memory to a virtual address.
  The addresses must be 4KB page aligned. 'num_pages' is the number of pages that need
  to be mapped.
*/
void memory_map(uint32_t phys_addr, const void* virt_addr, int num_pages)
{
  acquire_mutex(&vmm_mutex);

  for (int i = 0; i < num_pages; i++) {
    int pdir_idx = (uint32_t)virt_addr >> 22;
    /* Allocate a page table if needed */
    if (!page_dir[pdir_idx].fields.present) {
      uint32_t ptbl_addr = alloc_phys_page();
      new_paging_entry(&page_dir[pdir_idx], ptbl_addr, virt_addr >= KERNEL_START);
    }

    int ptbl_idx = (uint32_t)virt_addr >> 12;
    new_paging_entry(&page_tables[ptbl_idx], phys_addr, virt_addr >= KERNEL_START);

    /* invalidate TLB */
    invlpg(virt_addr);

    phys_addr += PAGE_SIZE;
    virt_addr += PAGE_SIZE;
  }

  release_mutex(&vmm_mutex);
}

/*
  Allocate a physical page at virtual address addr
*/
void alloc_virt_page(const void* addr)
{
  // Map a new page to addr
  // TODO: handle out of memory
  memory_map(alloc_phys_page(), addr, 1);
}

/*
  Free a page mapped at virtual address addr
*/
void free_virt_page(const void* addr)
{
  acquire_mutex(&vmm_mutex);

  paging_t* tbl = &page_tables[(uint32_t)addr >> 12];
  free_phys_page(tbl->word & 0xfffff000);
  tbl->word = 0;

  /* invalidate TLB */
  invlpg(addr);

  release_mutex(&vmm_mutex);
}

/*
  Virtual Memory Manager initialization
*/
static void vmm_init()
{
  /*
    We have already set up page directory and page tables in boot.S, but that covered
    even the unused parts of PMM bitmap. Let us free that here.
  */
  void* addr = pmm_bitmap + pmm_bitmap_length;
  /* Align addr to the next page boundary */
  addr += PAGE_SIZE - ((uint32_t)addr % PAGE_SIZE);
  /* Store this as the start address for kernel heap */
  extern void* KERNEL_HEAP_START;
  KERNEL_HEAP_START = addr;
  /* Now free all unused pages in [addr, KERNEL_END) */
  while (addr < KERNEL_END) {
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
