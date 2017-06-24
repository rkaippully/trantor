/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/


#ifndef _DRIVERS_H
#define _DRIVERS_H

/* Device driver interface */
typedef struct {
  void (*init)();
  void (*shutdown)();
} driver_t;

extern driver_t* drivers[];

#endif
