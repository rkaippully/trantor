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
#include "string.h"
#include "kernel/multiboot.h"
#include "kinit.h"

typedef struct {
  char filename[16];
  char unused[32];
  union {
    char filesize_str[10];
    uint32_t filesize;
  };
  uint16_t signature;
} __attribute__((packed)) fileheader_t;

static fileheader_t* files;

return_t load_initrd(multiboot_info_t* boot_info)
{
  if (boot_info->mods_count != 1)
    return INVALID_MOD_COUNT;

  multiboot_mod_t* mod = boot_info->mods_addr;
  if (mod->end_addr <= mod->start_addr + 8)
    return INITRD_FORMAT_ERROR;

  uint8_t* p = mod->start_addr;
  if (memcmp(p, "!<arch>\n", 8) != 0)
    return INITRD_FORMAT_ERROR;

  files = (fileheader_t*)(p + 8);

  return SUCCESS;
}
