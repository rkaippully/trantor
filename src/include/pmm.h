/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#ifndef _PMM_H
#define _PMM_H

/*
  Physical memory management

  PMM handles physical address space. It is responsible for allocating and
  releasing pages of physical memory that can then be used for paging, DMA etc.

  PMM stores information about the availability of physical memory in a bitmap
  located at 0xc8000000. Each set bit in the bitmap indicates a free page.
 */

#include "stdint.h"

extern uint8_t pmm_bitmap;

#endif
