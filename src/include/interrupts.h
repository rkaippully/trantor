/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#ifndef _INTERRUPTS_H
#define _INTERRUPTS_H

#include "stdint.h"

/* Well known IRQs */
extern void general_isr(void);
extern void divide_error_isr(void);
extern void debug_isr(void);
extern void nmi_isr(void);
extern void breakpoint_isr(void);
extern void overflow_isr(void);
extern void bound_range_isr(void);
extern void invalid_opcode_isr(void);
extern void no_coprocessor_isr(void);
extern void double_fault_isr(void);
extern void invalid_tss_isr(void);
extern void segment_not_present_isr(void);
extern void stack_fault_isr(void);
extern void general_protection_isr(void);
extern void page_fault_isr(void);
extern void math_fault_isr(void);
extern void align_check_isr(void);
extern void machine_check_isr(void);
extern void simd_fault_isr(void);

extern void nop_intr_handler();

/* A table of interrupt handlers */
extern void (*isr_funcs[256])(void);
extern void (*irq_funcs[16])(void);

extern void set_interrupt_gate(uint8_t index, void* func, bool is_user);

/* The IDT */
extern uint32_t idt[256 * 2];
extern uint16_t idt_descr[4];

extern void irq_0_isr(void);
extern void irq_1_isr(void);
extern void irq_2_isr(void);
extern void irq_3_isr(void);
extern void irq_4_isr(void);
extern void irq_5_isr(void);
extern void irq_6_isr(void);
extern void irq_7_isr(void);
extern void irq_8_isr(void);
extern void irq_9_isr(void);
extern void irq_10_isr(void);
extern void irq_11_isr(void);
extern void irq_12_isr(void);
extern void irq_13_isr(void);
extern void irq_14_isr(void);
extern void irq_15_isr(void);

#endif
