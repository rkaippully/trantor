/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#include "drivers.h"

extern Driver keyboard_driver;

// List of all device drivers
Driver* drivers [] = {
  &keyboard_driver,
  0
};

void init_drivers()
{
  for (Driver** d = drivers; *d != 0; d++)
    (*d)->init();
}
