/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/


#ifndef _VMM_H
#define _VMM_H

/* Virtual memory management */

#include "stdint.h"

/*
  Allocate a virtual page at the specified address. Returns the allocation
  status.
*/
extern bool vmm_alloc(uint32_t addr);

/*
  Frees a previously allocated physical page. Returns the status of operation.
*/
extern bool vmm_free(uint32_t addr);

#endif
