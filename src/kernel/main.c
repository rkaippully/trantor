/*
  Trantor Operating System

  Copyright (C) 2014 Raghu Kaippully
*/

extern void mm_init();
extern void pic_init();
extern void interrupts_init();
extern void timer_init();

void kernel_main()
{
  mm_init();
  pic_init();
  timer_init();
  interrupts_init();
}
