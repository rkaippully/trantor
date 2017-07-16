/*
	Trantor Operating System

	Copyright (C) 2017 Raghu Kaippully

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.

  This Source Code Form is "Incompatible With Secondary Licenses", as
	defined by the Mozilla Public License, v. 2.0.
*/

#include "string.h"

int memcmp(const void* s1, const void* s2, size_t n)
{
  const unsigned char* c1 = s1;
  const unsigned char* c2 = s2;
  while (n-- > 0) {
    if (*c1 != *c2)
      return *c1 - *c2;
    c1++;
    c2++;
  }

  return 0;
}

void* memchr(const void *s, int c, size_t n)
{
  const unsigned char* s1 = s;
  unsigned char c1 = c;
  while (n-- > 0) {
    if (*s1 == c1)
      return (void*)s1;
    s1++;
  }
  return 0;
}
