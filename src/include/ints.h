/*
  Trantor Operating System

  Copyright (C) 2014 Raghu Kaippully
*/

#ifndef _INTS_H
#define _INTS_H

/* Integer data types */
typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;

typedef signed char         int8_t;
typedef signed short        int16_t;
typedef signed int          int32_t;
typedef signed long long    int64_t;

typedef enum {
  false = 0,
  true = 1
} bool;

/* Find the ceiling of m divided by n */
#define ceil(m, n) ((m) % (n) ? ((m)/(n) + 1) : ((m)/(n)))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

#endif
