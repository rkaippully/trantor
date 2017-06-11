/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#ifndef _PRINTF_H
#define _PRINTF_H

#include "stdarg.h"
#include "asm.h"

static const int MAX_BUF_SIZE = 256;

#ifdef USE_E9_DEBUG
extern void kdebug(const char* format, ...);
#else
#define kdebug(format, ...)
#endif

#define kpanic(format, ...) do { kdebug(format, ##__VA_ARGS__); halt(); } while(0)

extern void vsprintf(char* buf, const char* fmt, va_list args);
extern void sprintf(char* buf, const char* fmt, ...);
extern void cprintf(const char* format, ...);

#endif
