/*
	Trantor Operating System

	Copyright (C) 2017 Raghu Kaippully

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.

  This Source Code Form is "Incompatible With Secondary Licenses", as
	defined by the Mozilla Public License, v. 2.0.
*/

#ifndef _MEMORY_H
#define _MEMORY_H

#define KERNEL_STACK_SIZE 4096

#define KERNEL_CODE_SELECTOR 0x0008
#define KERNEL_DATA_SELECTOR 0x0010
#define USER_CODE_SELECTOR   0x001b
#define USER_DATA_SELECTOR   0x0023

#ifndef __ASSEMBLER__

typedef uint32_t page_dir_t;
typedef uint32_t page_tbl_t;

extern page_tbl_t kernel_page_tbl[1024];

typedef struct {
  uint16_t limit_0;
  uint16_t base_0;
  uint8_t  base_1;
  unsigned type:4;
  unsigned system:1;
  unsigned dpl:2;
  unsigned present:1;
  unsigned limit_1:4;
  unsigned unused:2;
  unsigned opr_size:1;
  unsigned granularity:1;
  uint8_t  base_2;
} __attribute__((packed)) descriptor_t;

typedef struct {
  uint16_t      limit;
  descriptor_t* base;
} __attribute__((packed)) sys_tbl_reg_t;

extern uint8_t _kernel_start, _kernel_end;

#endif

#endif
