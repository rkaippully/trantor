/*
	Trantor Operating System

	Copyright (C) 2017 Raghu Kaippully

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.

  This Source Code Form is "Incompatible With Secondary Licenses", as
	defined by the Mozilla Public License, v. 2.0.
*/

#ifndef _STRING_H
#define _STRING_H

#include "stdtypes.h"

extern int memcmp(const void* s1, const void* s2, size_t n);
extern void* memchr(const void *s, int c, size_t n);

#endif
