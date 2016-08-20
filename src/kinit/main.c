/*
  Trantor Operating System

  Copyright (C) 2016 Raghu Kaippully
*/

/*
  This is the C entry point of kinit (called from boot.S).

  First we perform a necessary initializations to get the kernel running and
  then start the boot processes.
 */

extern void mm_init();
extern void proc_init();
extern void pic_init();
extern void interrupts_init();
extern void timer_init();

static void init()
{
  mm_init();
  proc_init();
  pic_init();
  timer_init();
  interrupts_init();
}

void kernel_main()
{
  init();
}
