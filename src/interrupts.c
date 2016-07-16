/*
  Trantor Operating System

  Copyright (C) 2014 Raghu Kaippully
*/

#include "ints.h"
#include "asm.h"

/*
  Interrupt and exception handling

  There are two types of interrupts - CPU generated interrupts and I/O device generated
  IRQs. Interrupt service routines for both these categories are defined in isr.S. Those
  assembly routines does some initial set up and then invoke the C functions defined in
  this file.
*/

void divide_error_isr()
{
  // TODO: Implement later
}

void nmi_isr()
{
  // TODO: Implement later
}

void breakpoint_isr()
{
  // TODO: Implement later
}

void overflow_isr()
{
  // TODO: Implement later
}

void bound_range_isr()
{
  // TODO: Implement later
}

void invalid_opcode_isr()
{
  // TODO: Implement later
}

void device_unavailable_isr()
{
  // TODO: Implement later
}

void double_fault_isr()
{
  // TODO: Implement later
}

void invalid_tss_isr()
{
  // TODO: Implement later
}

void segment_not_present_isr()
{
  // TODO: Implement later
}

void stack_fault_isr()
{
  // TODO: Implement later
}

void gp_fault_isr()
{
  // TODO: Implement later
}

void page_fault_isr()
{
  // TODO: Implement later
}

void fpu_error_isr()
{
  // TODO: Implement later
}

void alignment_check_isr()
{
  // TODO: Implement later
}

void machine_check_isr()
{
  // TODO: Implement later
}

/*
  Invoked by PIC IRQs
*/
void irq_isr(int irq, regs_t regs)
{
}

/*
  Structures for representing IDT
*/
typedef struct __attribute__((packed)) {
  uint16_t offset_low;
  uint16_t segment_selector;
  uint8_t  reserved;
  int      gate_type:4;
  int      descr_type:1;
  int      dpl:2;
  int      present:1;
  uint16_t offset_high;
} idt_entry;

/*
  Space for idt is allocated in boot.S
*/
extern idt_entry idt;

static struct __attribute__((packed)) {
  uint16_t limit;
  void* idt_base;
} idt_descr = {
  256*8 - 1,
  &idt
};

/*
  Creates an interrupt gate for the specified interrupt vector
*/
static void set_interrupt_gate(int vector, void (*isr)())
{
  const uint16_t KERNEL_CODE_SEG = 0x08;
  idt_entry* e = &idt + vector;

  e->offset_low = (uint32_t)(isr);
  e->segment_selector = KERNEL_CODE_SEG;
  e->reserved = 0;
  e->gate_type = 0x0e; // Interrupt gate type
  e->descr_type = 0;
  e->dpl = 0;
  e->present = 1;
  e->offset_high = ((uint32_t)(isr)) >> 16;
}

void interrupts_init()
{
  /* ISRs defined in isr.S */
  extern void nop_isr();
  extern void divide_error_isr_wrapper();
  extern void nmi_isr_wrapper();
  extern void breakpoint_isr_wrapper();
  extern void overflow_isr_wrapper();
  extern void bound_range_isr_wrapper();
  extern void invalid_opcode_isr_wrapper();
  extern void device_unavailable_isr_wrapper();
  extern void double_fault_isr_wrapper();
  extern void invalid_tss_isr_wrapper();
  extern void segment_not_present_isr_wrapper();
  extern void stack_fault_isr_wrapper();
  extern void gp_fault_isr_wrapper();
  extern void page_fault_isr_wrapper();
  extern void fpu_error_isr_wrapper();
  extern void alignment_check_isr_wrapper();
  extern void machine_check_isr_wrapper();
  extern void irq_0_isr_wrapper();
  extern void irq_1_isr_wrapper();
  extern void irq_2_isr_wrapper();
  extern void irq_3_isr_wrapper();
  extern void irq_4_isr_wrapper();
  extern void irq_5_isr_wrapper();
  extern void irq_6_isr_wrapper();
  extern void irq_7_isr_wrapper();
  extern void irq_8_isr_wrapper();
  extern void irq_9_isr_wrapper();
  extern void irq_a_isr_wrapper();
  extern void irq_b_isr_wrapper();
  extern void irq_c_isr_wrapper();
  extern void irq_d_isr_wrapper();
  extern void irq_e_isr_wrapper();
  extern void irq_f_isr_wrapper();

  /* Initialize everything with nop_isr */
  for (int i = 0; i < 256; i++)
    set_interrupt_gate(i, nop_isr);

  /* Initialize exceptions and interrupts */
  set_interrupt_gate(0, divide_error_isr_wrapper);
  set_interrupt_gate(2, nmi_isr_wrapper);
  set_interrupt_gate(3, breakpoint_isr_wrapper);
  set_interrupt_gate(4, overflow_isr_wrapper);
  set_interrupt_gate(5, bound_range_isr_wrapper);
  set_interrupt_gate(6, invalid_opcode_isr_wrapper);
  set_interrupt_gate(7, device_unavailable_isr_wrapper);
  set_interrupt_gate(8, double_fault_isr_wrapper);
  set_interrupt_gate(10, invalid_tss_isr_wrapper);
  set_interrupt_gate(11, segment_not_present_isr_wrapper);
  set_interrupt_gate(12, stack_fault_isr_wrapper);
  set_interrupt_gate(13, gp_fault_isr_wrapper);
  set_interrupt_gate(14, page_fault_isr_wrapper);
  set_interrupt_gate(16, fpu_error_isr_wrapper);
  set_interrupt_gate(17, alignment_check_isr_wrapper);
  set_interrupt_gate(18, machine_check_isr_wrapper);

  /* Initialize IRQs */
  set_interrupt_gate(0x80, irq_0_isr_wrapper);
  set_interrupt_gate(0x81, irq_1_isr_wrapper);
  set_interrupt_gate(0x82, irq_2_isr_wrapper);
  set_interrupt_gate(0x83, irq_3_isr_wrapper);
  set_interrupt_gate(0x84, irq_4_isr_wrapper);
  set_interrupt_gate(0x85, irq_5_isr_wrapper);
  set_interrupt_gate(0x86, irq_6_isr_wrapper);
  set_interrupt_gate(0x87, irq_7_isr_wrapper);
  set_interrupt_gate(0x88, irq_8_isr_wrapper);
  set_interrupt_gate(0x89, irq_9_isr_wrapper);
  set_interrupt_gate(0x8a, irq_a_isr_wrapper);
  set_interrupt_gate(0x8b, irq_b_isr_wrapper);
  set_interrupt_gate(0x8c, irq_c_isr_wrapper);
  set_interrupt_gate(0x8d, irq_d_isr_wrapper);
  set_interrupt_gate(0x8e, irq_e_isr_wrapper);
  set_interrupt_gate(0x8f, irq_f_isr_wrapper);

  /* Load the IDT and enable all interrupts */
  __asm__ volatile("lidt %0; sti" :: "m"(idt_descr));
}
