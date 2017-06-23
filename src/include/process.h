/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#ifndef _PROCESS_H
#define _PROCESS_H

#include "stdint.h"

typedef enum {
  READY, RUNNING, BLOCKED
} proc_state_t;

struct mutex_t;

typedef struct process_t {
  uint32_t     pid;
  proc_state_t state;

  /* Saved state during task switch */
  uint32_t     esp, cr3;

  /* The mutex this process is blocked on */
  struct mutex_t*   blocked_on;
  /* Linked list of mutex waiters */
  struct process_t* next_waiter;

  /* Linked list of ready processes */
  struct process_t* next_ready;
  struct process_t* prev_ready;
} process_t;

extern volatile process_t* current_proc;

typedef struct mutex_t {
  // Owning process of this mutex or NULL
  process_t* owner;
  // Processes waiting on this mutex
  process_t* waiters;
} mutex_t;

#define INIT_MUTEX { .owner = 0, .waiters = 0 }

extern void acquire_mutex(mutex_t* m);
extern void release_mutex(mutex_t* m);

/*
  Yield the current time slice to the process owning mutex m. When yield()
  returns, the current process will be owning mutex m.
*/
extern void yield(mutex_t* m);

/* Move a processs to READY state */
extern void proc_mark_ready(process_t* p);

extern void proc_schedule();

#endif
