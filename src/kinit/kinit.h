/*
	Trantor Operating System

	Copyright (C) 2017 Raghu Kaippully

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.

  This Source Code Form is "Incompatible With Secondary Licenses", as
	defined by the Mozilla Public License, v. 2.0.
*/

#ifndef _KINIT_H
#define _KINIT_H

#include "kernel/multiboot.h"

// Error codes
typedef enum {
  SUCCESS = 0,
  INVALID_BOOT_SIG,
  INVALID_BOOT_FLAGS,
  INVALID_MOD_COUNT,
  INITRD_FORMAT_ERROR,
} return_t;

typedef struct initrd_file_t initrd_file_t;

extern return_t load_initrd(multiboot_info_t* boot_info);
extern const initrd_file_t* get_initrd_files();
extern const initrd_file_t* next_initrd_file(const initrd_file_t* f);

extern return_t init_memory();

#endif
