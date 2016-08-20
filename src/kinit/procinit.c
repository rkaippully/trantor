/*
  Trantor Operating System

  Copyright (C) 2016 Raghu Kaippully
*/

#include "mm.h"
#include "mminternal.h"
#include "proc.h"

void proc_init()
{
  proc_t* init = kmalloc(sizeof(proc_t));
  init->pid = 1;
  init->ppid = 0;
  init->next = 0;
  procs = init;

  tss.iomap_base = (void*)tss.iopb - (void*)&tss;
  tss.ss0 = KERNEL_DS_SEL;

  extern uint8_t kernel_stack_end;
  tss.esp0 = (uint32_t)&kernel_stack_end;

  // Load TSS into TR
  gdt[5] = make_gdt_descriptor((uint32_t)&tss, sizeof(tss), 0, 0x09);
  __asm__ volatile("ltrw %0" :: "m"(TSS_SEL));

  // Set TS flag in CR0
  uint32_t cr0;
  __asm__ volatile(
    "movl   %%cr0, %0     ;"
    "orl    $8, %0        ;"
    "movl   %0, %%cr0     ;"
    : "=r"(cr0));
}
