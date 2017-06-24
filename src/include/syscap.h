/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

/* System capabilities detected */

#ifndef _SYSCAP_H
#define _SYSCAP_H

#include "stdint.h"

typedef struct {
  bool x87;
  bool vme;
  bool pge;
  bool mmx;
  bool fxsave;
} syscap_t;

extern syscap_t syscap;

#endif
