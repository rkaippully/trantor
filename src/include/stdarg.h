/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#ifndef _STDARG_H
#define _STDARG_H

typedef char* va_list;

/* Guarentees that type takes at least sizeof(void*) bytes in the stack */
#define _va_round(type) ((sizeof(type) + sizeof(void*) - 1) & ~(sizeof(void*) - 1))

#define va_start(ap, paramN) ((ap) = (char*)&paramN + (_va_round(paramN)))
#define va_end(ap) ((ap) = (void*)0)
#define va_arg(ap, type) ((ap) += (_va_round(type)), (*(type*) ((ap) - (_va_round(type)))))

#endif
