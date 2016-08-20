/*
  Trantor Operating System

  Copyright (C) 2016 Raghu Kaippully
*/

#include "proc.h"
#include "mm.h"

proc_t* procs = 0;
thread_t* threads = 0;

volatile thread_t* running_thread = 0;

tss_t tss;

/*
  Initialize a new task by setting up the user address space and handing off control to
  it.
*/
static void init_new_thread(thread_t* thread)
{
  uint16_t seg_sel = 0;
  __asm__ volatile(
    "movw   %1, %0    ;"
    "movw   %0, %%ds  ;"
    "movw   %0, %%es  ;"
    "movw   %0, %%fs  ;"
    "movw   %0, %%gs  ;"
    "pushl  %1        ;"  /* ss */
    "pushl  %2        ;"  /* esp */
    "pushl  %3        ;"  /* eflags */
    "pushl  %4        ;"  /* cs */
    "pushl  %5        ;"  /* eip */
    "iret"
    : "+r"(seg_sel) : "i"(USER_DS_SEL), "i"(PAGE_SIZE), "i"(0x200), "i"(USER_CS_SEL), "i"(0));
}

/*
  Initialize a kernel stack for a new thread.
*/
static void inline init_new_kernel_stack(volatile thread_t* thread)
{
  /* Map the kernel stack */
  memory_map(thread->kstack_page, ksstart, 1);

  /* Now we need to put some dummy values in the stack so that when the dispatcher
     returns it will unwind properly. */
  extern uint32_t _leave_irq_isr;
  __asm__ volatile(
    "movl   %0, %%esp       ;"
    "pushl  $0              ;"     /* dummy error code */
    "pushl  $0x200          ;"     /* eflags */
    "pushl  %1              ;"     /* cs */
    "pushl  %2              ;"     /* eip = init_new_thread */
    "pushl  $0              ;"     /* eax */
    "pushl  $0              ;"     /* ecx */
    "pushl  $0              ;"     /* edx */
    "pushl  $0              ;"     /* ebx */
    "pushl  $0              ;"     /* esp */
    "pushl  $0              ;"     /* ebp */
    "pushl  $0              ;"     /* esi */
    "pushl  $0              ;"     /* edi */
    "pushl  %3              ;"     /* ds */
    "pushl  %3              ;"     /* es */
    "pushl  %3              ;"     /* fs */
    "pushl  %3              ;"     /* gs */
    "pushl  $0              ;"     /* IRQ number */
    "pushl  %4              ;"     /* return address to timer_isr */
    :: "i"(ksend),
     "i"(KERNEL_CS_SEL), "i"(&init_new_thread), "i"(KERNEL_DS_SEL), "i"(&_leave_irq_isr));
}

/* The dispatcher - invoked by a timer interrupt and switches to the next ready thread. */
void task_dispatch()
{
  if (threads && (running_thread == 0 || running_thread != running_thread->next)) {
    /* Time slice expired and we may have other ready threads */
    static volatile thread_t* to_run;
    to_run = running_thread ? running_thread->next : threads;
    while (to_run->state > RUNNING)
      to_run = running_thread->next;

    if (running_thread != 0)
      running_thread->state = READY;

    /* Switch kernel stack and cr3, and voila we are in a new task */
    uint32_t curr_cr3 = 0;
    __asm__ volatile(
      // Save current ESP if we are running a thread
      "testl %2, %2      ;"
      "jz    1f          ;"
      "movl  %%esp, %0   ;"
      "1:                 "
      // Set new thread's ESP
      "movl  %3, %%esp   ;"
      // Switch CR3 if we are moving to a new process
      "movl  %%cr3, %1   ;"
      "cmpl  %1, %4      ;"
      "je    1f          ;"
      "movl  %4, %%cr3   ;"
      "1:                "
      : "=m"(running_thread->esp0), "+r"(curr_cr3)
      : "r"(running_thread), "m"(to_run->esp0), "r"(to_run->proc->cr3));

    /*
      DO NOT use any stack variables below this. We could be in a new task with no
      stack set up.
    */
    if (to_run->state == NEW)
      init_new_kernel_stack(to_run);

    running_thread = to_run;
    to_run->state = RUNNING;
  }
}
