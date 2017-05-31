/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#ifndef _DEBUG_H
#define _DEBUG_H

#ifdef USE_E9_DEBUG
extern void kdebug(const char* format, ...);
#else
#define kdebug(format, ...)
#endif

#endif
