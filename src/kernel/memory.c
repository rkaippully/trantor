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

uint8_t kernel_stack[KERNEL_STACK_SIZE] __attribute__((aligned(4096)));

page_tbl_t kernel_page_tbl[1024] __attribute__((aligned(4096)));

#define GDT_SIZE 6

static descriptor_t gdt[GDT_SIZE] __attribute__((aligned(8))) = {
  {0,0,0,0,0,0,0,0,0,0,0,0},
  // Kernel code segment
  {
    .limit_0     = 0xffff,
    .base_0      = 0x0000,
    .base_1      = 0x00,
    .type        = 0xa,
    .system      = 0x1,
    .dpl         = 0x0,
    .present     = 0x1,
    .limit_1     = 0xf,
    .unused      = 0x0,
    .opr_size    = 0x1,
    .granularity = 0x1,
    .base_2      = 0x00
  },
  // Kernel data segment
  {
    .limit_0     = 0xffff,
    .base_0      = 0x0000,
    .base_1      = 0x00,
    .type        = 0x2,
    .system      = 0x1,
    .dpl         = 0x0,
    .present     = 0x1,
    .limit_1     = 0xf,
    .unused      = 0x0,
    .opr_size    = 0x1,
    .granularity = 0x1,
    .base_2      = 0x00
  },
  // User code segment
  {
    .limit_0     = 0xffff,
    .base_0      = 0x0000,
    .base_1      = 0x00,
    .type        = 0xa,
    .system      = 0x1,
    .dpl         = 0x3,
    .present     = 0x1,
    .limit_1     = 0xf,
    .unused      = 0x0,
    .opr_size    = 0x1,
    .granularity = 0x1,
    .base_2      = 0x00
  },
  // User data segment
  {
    .limit_0     = 0xffff,
    .base_0      = 0x0000,
    .base_1      = 0x00,
    .type        = 0x2,
    .system      = 0x1,
    .dpl         = 0x3,
    .present     = 0x1,
    .limit_1     = 0xf,
    .unused      = 0x0,
    .opr_size    = 0x1,
    .granularity = 0x1,
    .base_2      = 0x00
  }
};

sys_tbl_reg_t gdt_descriptor = {
  .limit = GDT_SIZE*sizeof(descriptor_t) - 1,
  .base  = gdt
};
