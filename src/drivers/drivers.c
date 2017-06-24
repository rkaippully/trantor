/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#include "drivers.h"

extern driver_t keyboard_driver;

// List of all device drivers
driver_t* drivers [] = {
  &keyboard_driver,
  0
};

void init_drivers()
{
  for (driver_t** d = drivers; *d != 0; d++)
    (*d)->init();
}
