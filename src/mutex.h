/*
    Trantor Operating System
    Copyright (C) 2014 Raghu Kaippully

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _MUTEX_H
#define _MUTEX_H

/*
    A mutex - 0 = not acquired, 1 = acquired
*/
typedef int mutex_t;

static inline void acquire_mutex(mutex_t* m)
{
    int val = 1;
    __asm__ volatile(
        "1:                  "
        "   cmpl    $0, %0  ;"  /* test if lock is available */
        "   je      2f      ;"
        "   pause           ;"  /* recommended for Intel P4 & above */
        "   jmp     1b      ;"  /* spin if lock is not available */
        "2:                  "
        "   xchgl   %1, %0  ;"  /* try to get lock */
        "   cmpl    $0, %1  ;"  /* check if successful */
        "   jne 1b          "
        : "+m"(*m), "+q"(val));
}

static inline void release_mutex(mutex_t* m)
{
    *m = 0;
}

#endif

/* vim: set expandtab ai nu ts=4 tw=90: */
