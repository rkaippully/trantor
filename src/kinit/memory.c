/*
	Trantor Operating System

	Copyright (C) 2017 Raghu Kaippully

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.

  This Source Code Form is "Incompatible With Secondary Licenses", as
	defined by the Mozilla Public License, v. 2.0.
*/

#include "stdtypes.h"
#include "kernel/memory.h"
#include "kinit.h"

extern uint8_t _kinit_start, _kinit_end;

page_dir_t page_dir[1024] __attribute__((aligned(4096)));
page_tbl_t kinit_page_tbl[1024] __attribute__((aligned(4096)));

static inline void* physical_addr_of(void* addr)
{
  return (uint8_t*)addr - &_kernel_start + &_kinit_end;
}

return_t init_memory()
{
  /* Identity map kinit memory page table */
  uint32_t addr = (uint32_t)&_kinit_start;
  page_dir[addr >> 22] = (uint32_t)kinit_page_tbl + 7;
  while (addr < (uint32_t)&_kinit_end) {
    int ptbl_idx = (addr >> 12) & 0x3ff;
    kinit_page_tbl[ptbl_idx] = addr + 7;
    addr += 0x1000;
  }

  /* Map kernel page table */
  addr = (uint32_t)&_kernel_start;
  page_tbl_t* ptbl = physical_addr_of(kernel_page_tbl);
  page_dir[addr >> 22] = (uint32_t)ptbl + 3;
  while (addr < (uint32_t)&_kernel_end) {
    int ptbl_idx = (addr >> 12) & 0x3ff;
    ptbl[ptbl_idx] = (page_tbl_t)physical_addr_of((void*)(addr + 3));
    addr += 0x1000;
  }

  /* Enable paging and reinit segment registers */
  uint32_t tmp32;
  uint16_t tmp16;
  __asm__ volatile("movl  %2, %0              ;"
                   "movl  %0, %%cr3           ;"
                   "movl  %%cr0, %0           ;"
                   "btsl  $31, %0             ;"
                   "movl  %0, %%cr0           ;"
                   "lgdtl gdt_descriptor      ;"
                   "ljmp  %3, $1f             ;"
                   "1:                        ;"
                   "movw  %4, %1              ;"
                   "movw  %1, %%ds            ;"
                   "movw  %1, %%es            ;"
                   "movw  %1, %%fs            ;"
                   "movw  %1, %%gs            ;"
                   "movw  %1, %%ss            ;"
                   "movl  %%esp, %0           ;"
                   "subl  $_kinit_end, %0     ;"
                   "addl  $_kernel_start, %0  ;"
                   "movl  %0, %%esp           ;"
                   : "=r"(tmp32), "=r"(tmp16)
                   : "i"(&page_dir), "i"(KERNEL_CODE_SELECTOR), "i"(KERNEL_DATA_SELECTOR));

  return SUCCESS;
}
