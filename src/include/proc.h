/*
  Trantor Operating System

  Copyright (C) 2016 Raghu Kaippully
*/

#include "ints.h"
#include "mminternal.h"

/*
  The process and thread control structure stores all information about
  processes and threads.

  For now, we have a simple round robin scheduler with no priorities.
*/
typedef uint32_t pid_t;
typedef uint32_t tid_t;

typedef struct _proc_t proc_t;
typedef struct _thread_t thread_t;

struct _proc_t {
  pid_t pid, ppid;
  uint32_t cr3;
  proc_t* next;
};

// Linked list of all processes
extern proc_t* procs;
extern thread_t* threads;

typedef enum {
  NEW,
  READY,
  RUNNING,
  BLOCKED,
  TERMINATED
} tstate_t;

struct _thread_t {
  tid_t tid;
  proc_t* proc;
  tstate_t state;
  thread_t* next;
  uint32_t esp0;
  phys_page kstack_page;
};

// Currently running thread
extern volatile thread_t* running_thread;

/* We use software task switching but still need one TSS. */
typedef struct _tss_t {
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

extern tss_t tss;

extern uint8_t kernel_stack_start, kernel_stack_end;
static const void* const ksstart = &kernel_stack_start;
static const void* const ksend = &kernel_stack_end;
