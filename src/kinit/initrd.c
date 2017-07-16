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

typedef struct initrd_file_t {
  char filename[16];
  char unused[32];
  union {
    char filesize_str[10];
    uint32_t filesize;
  } __attribute__((packed));
  uint16_t signature;
} __attribute__((packed)) initrd_file_t;

static initrd_file_t* files;
static initrd_file_t* last_file;

return_t load_initrd(multiboot_info_t* boot_info)
{
  if (boot_info->mods_count != 1)
    return INVALID_MOD_COUNT;

  multiboot_mod_t* mod = boot_info->mods_addr;
  if (mod->end_addr <= mod->start_addr + 8 + sizeof(initrd_file_t))
    return INITRD_FORMAT_ERROR;

  uint8_t* s = mod->start_addr;
  if (memcmp(s, "!<arch>\n", 8) != 0)
    return INITRD_FORMAT_ERROR;

  files = (initrd_file_t*)(s + 8);
  for (initrd_file_t* p = files; p < (initrd_file_t*)mod->end_addr; ) {
    // Make sure the file name ends with a '/'
    char* cp = memchr(p->filename, '/', 16);
    if (!cp)
      return INITRD_FORMAT_ERROR;
    *cp = 0;

    // Convert the file length
    uint32_t size = 0;
    for (int i = 0; i < 10; i++) {
      char c = p->filesize_str[i];
      if (c == ' ')
        break;
      if (c < '0' || c > '9')
        return INITRD_FORMAT_ERROR;
      uint32_t new_size = size*10 + c - '0';
      if (new_size < size)
        return INITRD_FORMAT_ERROR;
      size = new_size;
    }
    p->filesize = size;

    if (p->signature != 0x0a60)
      return INITRD_FORMAT_ERROR;

    last_file = p;
    p = (initrd_file_t*)((uint8_t*)p + p->filesize + sizeof(initrd_file_t));
  }

  return SUCCESS;
}

const initrd_file_t* get_initrd_files()
{
  return files;
}

const initrd_file_t* next_initrd_file(const initrd_file_t* f)
{
  if (f == 0 || f == last_file)
    return 0;

  uint8_t* p = (uint8_t*)f;
  return (initrd_file_t*)(p + sizeof(initrd_file_t) + f->filesize);
}
