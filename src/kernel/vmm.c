/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#include "asm.h"
#include "pmm.h"
#include "vmm.h"

typedef uint32_t page_dir_t;
typedef uint32_t page_tbl_t;

static page_dir_t* page_dir = (page_dir_t*)0xfffff000;
static page_tbl_t* page_tbls = (page_tbl_t*)0xffc00000;

static bool is_kernel(uint32_t addr)
{
  return addr >= 0xc0000000;
}

static inline void invlpg(uint32_t addr)
{
  __asm__ volatile("invlpg (%0)" : : "g"(addr));
}

static inline void zero_page(uint32_t addr)
{
  /* zero the page */
  __asm__ volatile(
    "movl  $1024, %%ecx    ;"
    "xorl  %%eax, %%eax    ;"
    "movl  %0, %%edi       ;"
    "cld                   ;"
    "rep stosl             ;"
    :
    : "g"(addr)
    : "eax", "ecx", "edi");
}

bool vmm_alloc(uint32_t addr)
{
  int d_idx = addr >> 22;
  int t_idx = addr >> 12;

  page_dir_t d_entry = page_dir[d_idx];
  if (!(d_entry & 1)) {
    page_dir_t addr = pmm_alloc();
    // TODO: Handle pmm allocation failure
    page_dir[d_idx] = addr | is_kernel(addr) ? 0x103 : 0x107;
    for (uint32_t i = 0; i < 1024; i++) {
      uint32_t idx = (d_idx << 10) + i;
      page_tbls[idx] = 0;
      invlpg(idx << 12);
    }
  }

  if (page_tbls[t_idx] & 1) {
    // This address is already allocated
    return false;
  }
  else {
    page_tbl_t addr = pmm_alloc();
    // TODO: Handle pmm allocation failure
    page_tbls[t_idx] = addr | is_kernel(addr) ? 0x103 : 0x107;
    invlpg(t_idx << 12);
    zero_page(addr);
    return true;
  }
}

bool vmm_free(uint32_t addr)
{
  int d_idx = addr >> 22;
  int t_idx = addr >> 12;

  page_dir_t d_entry = page_dir[d_idx];
  if (!(d_entry & 1)) {
    // No page directory entry for this address
    return false;
  }

  page_tbl_t entry = page_tbls[t_idx];
  if (entry & 1) {
    pmm_free(entry & 0xfffff000);
    page_tbls[t_idx] = 0;
    invlpg(t_idx << 12);
    return true;
  }
  else {
    // No page table entry for this address
    return false;
  }
}
