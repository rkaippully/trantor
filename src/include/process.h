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

typedef struct {
  uint16_t    prev_task;
  uint16_t    reserved1;
  uint32_t    esp0;
  uint16_t    ss0;
  uint16_t    reserved2;
  uint32_t    esp1;
  uint16_t    ss1;
  uint16_t    reserved3;
  uint32_t    esp2;
  uint16_t    ss2;
  uint16_t    reserved4;
  uint32_t    cr3;
  uint32_t    eip;
  uint32_t    eflags;
  uint32_t    eax;
  uint32_t    ecx;
  uint32_t    edx;
  uint32_t    ebx;
  uint32_t    esp;
  uint32_t    ebp;
  uint32_t    esi;
  uint32_t    edi;
  uint16_t    es;
  uint16_t    reserved5;
  uint16_t    cs;
  uint16_t    reserved6;
  uint16_t    ss;
  uint16_t    reserved7;
  uint16_t    ds;
  uint16_t    reserved8;
  uint16_t    fs;
  uint16_t    reserved9;
  uint16_t    gs;
  uint16_t    reserved10;
  uint16_t    ldt_selector;
  uint16_t    reserved11;
  uint16_t    reserved12;
  uint16_t    iomap_base;
  uint8_t     intr_redir[32];
  uint8_t     iopb[8193];
} __attribute__((packed)) tss_t;

/* We use software task switching but still need one TSS. */
extern tss_t tss;

#endif
