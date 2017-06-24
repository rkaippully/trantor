/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#include "asm.h"
#include "process.h"
#include "pmm.h"

static process_t idle_proc;

static void init_tss()
{
  tss.iomap_base = (void*)tss.iopb - (void*)&tss;
  tss.ss0 = KERNEL_DS;
  tss.esp0 = (uint32_t)KERNEL_STACK_END;

  uint16_t tss_idx = TSS_SELECTOR >> 3;
  uint32_t base = (uint32_t)&tss;
  uint32_t limit = sizeof(tss_t);
  uint64_t descr = ((base >> 24) << 8) + 0x40 + ((limit >> 16) & 0x0f);
  descr <<= 16;
  descr += 0x8900 + ((base >> 16) & 0xff);
  descr <<= 16;
  descr += base & 0xffff;
  descr <<= 16;
  descr += limit & 0xffff;
  gdt[tss_idx] = descr;

  uint16_t tss_selector = TSS_SELECTOR;
  __asm__ volatile("ltr %0" : : "r"(tss_selector));

  /* Set TS flag in CR0 */
  uint32_t cr0;
  __asm__ volatile(
    "movl   %%cr0, %0     ;"
    "orl    $8, %0        ;"
    "movl   %0, %%cr0     ;"
    : "=r"(cr0));
}

static void init_scheduler()
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

void init_proc()
{
  init_tss();
  init_scheduler();
}
