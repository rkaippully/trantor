/*
    Trantor Operating System
    Copyright (C) 2014 Raghu Kaippully

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _MM_H
#define _MM_H

#include "ints.h"

/* Size of a memory page - physical and virtual */
static const int PAGE_SIZE = 4096;

extern void memory_map(uint32_t phys_addr, const void* virt_addr, int num_pages);
extern void alloc_virt_page(const void* addr);
extern void free_virt_page(const void* addr);

#endif
