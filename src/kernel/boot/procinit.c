/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#include "process.h"

static process_t idle_proc;

void init_proc()
{
  uint32_t cr3;
  __asm__ volatile("movl  %%cr3, %0 ;" : "=r"(cr3));

  /* Create the idle process */
  process_t* p = &idle_proc;
  p->pid = 0;
  p->state = RUNNING;
  p->esp = 0;
  p->cr3 = cr3;
  p->blocked_on = 0;
  p->next_waiter = 0;
  p->next_ready = p;
  p->prev_ready = p;

  extern process_t* ready_procs;
  ready_procs = p;
  current_proc = p;
}
