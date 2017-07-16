/*
	Trantor Operating System

	Copyright (C) 2017 Raghu Kaippully

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.

  This Source Code Form is "Incompatible With Secondary Licenses", as
	defined by the Mozilla Public License, v. 2.0.
*/

#include "stdtypes.h"

uint16_t to_bcd(uint8_t x)
{
  uint16_t low = x & 0x0f;
  low += low < 10 ? '0' : ('a' - 10);
  uint16_t high = (x & 0xf0) >> 4;
  high += high < 10 ? '0' : ('a' - 10);
  return (high << 16) | low;
}
