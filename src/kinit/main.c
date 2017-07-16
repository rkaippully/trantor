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

return_t kmain(uint32_t boot_sig, multiboot_info_t* boot_info)
{
  if (boot_sig != 0x2badb002)
    return INVALID_BOOT_SIG;
  if ((boot_info->flags & 0x48) != 0x48)
    return INVALID_BOOT_FLAGS;

  return
    load_initrd(boot_info) ||
    init_memory();
}
