/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#ifndef _KMALLOC_H
#define _KMALLOC_H

#include "stdint.h"

extern void* kmalloc(uint32_t size);
extern void kfree(void* block);

#endif
