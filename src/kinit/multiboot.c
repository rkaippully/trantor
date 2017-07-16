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

/* Multiboot header */
struct {
  uint32_t magic;
  uint32_t flags;
  uint32_t checksum;
  uint32_t header_addr;
  uint32_t load_addr;
  uint32_t load_end_addr;
  uint32_t bss_end_addr;
  uint32_t entry_addr;
  uint32_t mode_type;
  uint32_t width;
  uint32_t height;
  uint32_t depth;
} __attribute__((packed)) multiboot_header __attribute__((section(".multiboot"))) = {
  .magic = 0x1badb002,
  .flags = 0x00000007,
  .checksum = 0xe4524ff7,
  .header_addr = 0,
  .load_addr = 0,
  .bss_end_addr = 0,
  .entry_addr = 0,
  .mode_type = 1,
  .width = 80,
  .height = 25,
  .depth = 0
};
