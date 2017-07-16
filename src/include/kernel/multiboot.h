/*
	Trantor Operating System

	Copyright (C) 2017 Raghu Kaippully

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.

  This Source Code Form is "Incompatible With Secondary Licenses", as
	defined by the Mozilla Public License, v. 2.0.
*/

#ifndef _MULTIBOOT_H
#define _MULTIBOOT_H

#include "stdtypes.h"

typedef struct {
  uint8_t* start_addr;
  uint8_t* end_addr;
  const char* cmdline;
  uint32_t reserved;
} __attribute__((packed)) multiboot_mod_t;

typedef struct {
  uint64_t base;
  uint64_t length;
  uint32_t type;
} __attribute__((packed)) multiboot_mmap_t;

typedef struct {
  uint32_t flags;
  uint32_t mem_lower, mem_upper;
  uint32_t boot_dev;
  uint32_t cmdline;
  uint32_t mods_count;
  multiboot_mod_t* mods_addr;
  uint32_t syms[4];
  uint32_t mmap_length;
  multiboot_mmap_t* mmap_addr;
} __attribute__((packed)) multiboot_info_t;

#endif
