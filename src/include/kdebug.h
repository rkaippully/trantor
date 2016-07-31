/*
  Trantor Operating System

  Copyright (C) 2016 Raghu Kaippully
*/

#ifndef _KDEBUG_H
#define _KDEBUG_H

#ifdef USE_BOCHS_E9
extern void kdebug(const char* format, ...);
#else
#define kdebug(fmt,...)
#endif

#endif
