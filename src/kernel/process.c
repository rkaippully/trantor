/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#include "process.h"
#include "asm.h"
#include "printf.h"

volatile process_t* current_proc = 0;

process_t* ready_procs = 0;

void proc_mark_ready(process_t* p)
{
  if (p->state == READY)
    kpanic("proc_mark_ready(): I am already ready!\n");
  p->state = READY;
  if (ready_procs) {
    p->next_ready = ready_procs;
    p->prev_ready = ready_procs->prev_ready;
  } else {
    p->next_ready = p->prev_ready = p;
  }
  ready_procs = p;
}

/*
  Recursively find the process to yield to based on mutex locks
*/
static process_t* get_yieldee(mutex_t* m)
{
  while (true) {
    if (!m)
      kpanic("yield(): Blocked process has no mutex\n");
    process_t* p = m->owner;
    if (!p)
      kpanic("yield(): Mutex has no owner\n");
    if (p->state == BLOCKED)
      m = p->blocked_on;
    else if (p->state != READY)
      kpanic("yield(): Deadlock!\n");
    else
      return p;
  }
}

/*
  Switch context from the current process to the specified process.
*/
static inline void switch_context(process_t* p)
{
  process_t* me = (process_t*)current_proc;
  current_proc = p;
  p->state = RUNNING;

  /* Switch stack and cr3, and we are in a new process */
  __asm__ volatile(
    "movl   %%esp, %0   ;"
    "movl   %1, %%esp   ;"
    "movl   %2, %%cr3   ;"
    : "=m"(me->esp)
    : "m"(p->esp), "r"(p->cr3));
}

/*
  Yield the current time slice to the process owning mutex m. When yield()
  returns, the current process will be owning mutex m.
*/
void yield(mutex_t* m)
{
  process_t* p = get_yieldee(m);

  process_t* me = (process_t*)current_proc;
  if (ready_procs == me)
    ready_procs = me->next_ready == me ? 0 : me->next_ready;
  me->prev_ready->next_ready = me->next_ready;
  me->next_ready->prev_ready = me->prev_ready;
  me->next_ready = me->prev_ready = 0;

  me->state = BLOCKED;
  me->blocked_on = m;
  me->next_waiter = m->waiters;
  m->waiters = me;

  switch_context(p);
}

void proc_schedule()
{
  process_t* p = current_proc->next_ready;
  if (p == current_proc)
    return;

  current_proc->state = READY;
  switch_context(p);
}
