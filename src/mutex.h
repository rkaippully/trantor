/*
    Trantor Operating System

    Copyright (C) 2014 Raghu Kaippully
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
