/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#include "asm.h"
#include "printf.h"
#include "interrupts.h"

#define PIC1            0x20    /* IO base address for master PIC */
#define PIC2            0xA0    /* IO base address for slave PIC */
#define PIC1_COMMAND    PIC1
#define PIC1_DATA       (PIC1+1)
#define PIC2_COMMAND    PIC2
#define PIC2_DATA       (PIC2+1)

#define ICW1_ICW4       0x01  /* ICW4 (not) needed */
#define ICW1_INIT       0x10  /* Initialization - required! */
#define ICW4_8086       0x01  /* 8086/88 (MCS-80/85) mode */

static void remap_pic()
{
  uint8_t mask1, mask2;

  /* Save masks */
  mask1 = inb(PIC1_DATA);
  mask2 = inb(PIC2_DATA);

  /* Start initialization sequence */
  outb(PIC1_COMMAND, ICW1_INIT + ICW1_ICW4);
  io_wait();
	outb(PIC2_COMMAND, ICW1_INIT + ICW1_ICW4);
	io_wait();
  /* Define the PIC vectors. IRQ 0-7 is mapped to 0x78-0x7f and IRQ 8-15 is mapped to 0x70-0x77. */
	outb(PIC1_DATA, 0x78);
	io_wait();
	outb(PIC2_DATA, 0x70);
	io_wait();
  /* continue initialization sequence */
	outb(PIC1_DATA, 4);
	io_wait();
	outb(PIC2_DATA, 2);
	io_wait();

	outb(PIC1_DATA, ICW4_8086);
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();

  /* restore saved masks. */
	outb(PIC1_DATA, mask1);
	outb(PIC2_DATA, mask2);
}

/* Set up IDTR */
static void lidt(void* base, uint16_t limit)
{
  idt_descr[1] = limit;
  idt_descr[2] = ((uint32_t) base) & 0xffff;
  idt_descr[3] = ((uint32_t) base) >> 16;
  __asm__ volatile("lidt (%0)" : : "r"(&idt_descr[1]));
}

static void init_interrupt_handlers()
{
  for (int i = 0; i < 256; i++) {
    set_interrupt_gate(i, general_isr, false);
    isr_funcs[i] = nop_intr_handler;
  }

  /* Set up IDT entries */
  set_interrupt_gate(0, divide_error_isr, false);
  set_interrupt_gate(1, debug_isr, false);
  set_interrupt_gate(2, nmi_isr, false);
  set_interrupt_gate(3, breakpoint_isr, false);
  set_interrupt_gate(4, overflow_isr, false);
  set_interrupt_gate(5, bound_range_isr, false);
  set_interrupt_gate(6, invalid_opcode_isr, false);
  set_interrupt_gate(7, no_coprocessor_isr, false);
  set_interrupt_gate(8, double_fault_isr, false);
  set_interrupt_gate(10, invalid_tss_isr, false);
  set_interrupt_gate(11, segment_not_present_isr, false);
  set_interrupt_gate(12, stack_fault_isr, false);
  set_interrupt_gate(13, general_protection_isr, false);
  set_interrupt_gate(14, page_fault_isr, false);
  set_interrupt_gate(16, math_fault_isr, false);
  set_interrupt_gate(17, align_check_isr, false);
  set_interrupt_gate(18, machine_check_isr, false);
  set_interrupt_gate(19, simd_fault_isr, false);

  /* Set IRQ handlers */
  for (int i = 0x78; i < 0x80; i++)
    irq_funcs[i - 0x70] = low_irq_handler;
  for (int i = 0x70; i < 0x78; i++)
    irq_funcs[i - 0x70] = high_irq_handler;

  set_interrupt_gate(0x70, irq_0_isr, false);
  set_interrupt_gate(0x71, irq_1_isr, false);
  set_interrupt_gate(0x72, irq_2_isr, false);
  set_interrupt_gate(0x73, irq_3_isr, false);
  set_interrupt_gate(0x74, irq_4_isr, false);
  set_interrupt_gate(0x75, irq_5_isr, false);
  set_interrupt_gate(0x76, irq_6_isr, false);
  set_interrupt_gate(0x77, irq_7_isr, false);
  set_interrupt_gate(0x78, irq_8_isr, false);
  set_interrupt_gate(0x79, irq_9_isr, false);
  set_interrupt_gate(0x7a, irq_10_isr, false);
  set_interrupt_gate(0x7b, irq_11_isr, false);
  set_interrupt_gate(0x7c, irq_12_isr, false);
  set_interrupt_gate(0x7d, irq_13_isr, false);
  set_interrupt_gate(0x7e, irq_14_isr, false);
  set_interrupt_gate(0x7f, irq_15_isr, false);

  lidt(idt, 256*8 - 1);
}

void init_interrupts()
{
  remap_pic();
  init_interrupt_handlers();

  sti();
  kdebug("Interrupts enabled.\n");
}
