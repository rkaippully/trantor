/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#include "process.h"
#include "asm.h"
#include "printf.h"

void acquire_mutex(mutex_t* m)
{
  cli();
  if (m->owner == current_proc)
    kpanic("acquire_mutex(): called by owner!\n");
  do {
    if (m->owner)
      yield(m);
    else
      m->owner = (process_t*)current_proc;
  } while (m->owner != current_proc);
  sti();
}

void release_mutex(mutex_t* m)
{
  cli();
  if (m->owner != current_proc)
    kpanic("release_mutex(): called by non-owner!\n");
  m->owner = 0;
  for (process_t* p = m->waiters; p != 0; p = p->next_waiter)
    proc_mark_ready(p);
  m->waiters = 0;
  sti();
}
