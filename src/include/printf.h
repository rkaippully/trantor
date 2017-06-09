/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#ifndef _PRINTF_H
#define _PRINTF_H

#include "stdarg.h"

static const int MAX_BUF_SIZE = 256;

#ifdef USE_E9_DEBUG
extern void kdebug(const char* format, ...);
#else
#define kdebug(format, ...)
#endif

extern void sprintf(char* buf, const char* fmt, va_list args);
extern void cprintf(const char* format, ...);

#endif
