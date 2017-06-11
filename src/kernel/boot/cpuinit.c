/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#include "stdint.h"
#include "printf.h"
#include "asm.h"
#include "syscap.h"

static void stupid_cpu(const char* msg)
{
  cprintf("Incompatible CPU (%s). System halted.", msg);
  halt();
}

static void check_cpuid_support()
{
  uint32_t supported = 0;
  __asm__ volatile(
    "pushfl                      ;"
    "orl   $0x00200000, (%%esp)  ;"
    "popfl                       ;"
    "pushfl                      ;"
    "btrl  $21, (%%esp)          ;"
    "jnc   1f                    ;"
    "popfl                       ;"
    "pushfl                      ;"
    "btl   $21, (%%esp)          ;"
    "jc    1f                    ;"
    "movl  $1, %0                ;"
    "1:                          ;"
    "popfl                       ;"
    : "=r"(supported));

  if (!supported)
    stupid_cpu("CPUID instruction not supported");
}

static void detect_cpu_vendor()
{
  uint8_t vendor[13] = { [0 ... 12] = 0 };
  uint32_t max_id;

  __asm__ volatile(
    "xorl  %%eax, %%eax          ;"
    "cpuid                       ;"
    "movl  %%eax, %0             ;"
    "movl  %%ebx, %1             ;"
    "movl  %%edx, %2             ;"
    "movl  %%ecx, %3             ;"
    : "=r"(max_id), "=m"(*vendor), "=m"(*(vendor+4)), "=m"(*(vendor+8))
    :
    : "eax", "ebx", "ecx", "edx");

  if (max_id < 1)
    stupid_cpu("Feature information not available");

  cprintf("Detected CPU: %s\n", vendor);
}

static void detect_feature_support()
{
  uint32_t features_edx;
  __asm__ volatile(
    "xorl  %%eax, %%eax          ;"
    "incl  %%eax                 ;"
    "cpuid                       ;"
    : "=d"(features_edx)
    :
    : "eax", "ebx", "ecx");

  syscap.x87 = (features_edx & 1 << 0) != 0;
  syscap.vme = (features_edx & 1 << 1) != 0;
  syscap.pge = (features_edx & 1 << 13) != 0;
  syscap.mmx = (features_edx & 1 << 23) != 0;
  syscap.fxsave = (features_edx & 1 << 24) != 0;

  kdebug("CPU capabilities: 0x%08x\n", features_edx);
  kdebug("    x87: %d\n", syscap.x87);
  kdebug("    vme: %d\n", syscap.vme);
  kdebug("    pge: %d\n", syscap.pge);
  kdebug("    mmx: %d\n", syscap.mmx);
  kdebug(" fxsave: %d\n", syscap.fxsave);
}

void init_cpu()
{
  check_cpuid_support();
  detect_cpu_vendor();
  detect_feature_support();
}
