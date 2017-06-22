/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#include "asm.h"
#include "interrupts.h"

/* The IDT */
uint32_t idt[256 * 2] __attribute__((aligned(8)));
uint16_t idt_descr[4] __attribute__((aligned(4)));

/* A table of interrupt handlers */
void (*isr_funcs[256])(void);
void (*irq_funcs[16])(void);

/*
  An interrupt handler that does nothing.
*/
void nop_intr_handler()
{
}

void set_interrupt_gate(uint8_t index, void* func, bool is_user)
{
  uint32_t f = (uint32_t)func;
  idt[index*2] = (f & 0xffff) | (KERNEL_CS << 16);
  idt[index*2 + 1] = (is_user ? 0xee00 : 0x8e00) | (f & 0xffff0000);
}

__asm__(
  ".global isr_handler          ;"
  "isr_handler:                 ;"
  "pushal                       ;"
  "pushl  %gs                   ;"
  "pushl  %fs                   ;"
  "pushl  %ds                   ;"
  "pushl  %es                   ;"
  "pushl  %ebp                  ;"
  "movl   %esp, %ebp            ;"
  "movw   $0x0010, %ax          ;"
  "movw   %ax, %ds              ;"
  "movw   %ax, %es              ;"
  "movw   %ax, %fs              ;"
  "movw   %ax, %gs              ;"
  "movl   52(%esp), %eax        ;" // interrupt vector
  "call   *_isr_funcs(,%eax,4)  ;"
  "leave                        ;"
  "popl  %es                    ;"
  "popl  %ds                    ;"
  "popl  %fs                    ;"
  "popl  %gs                    ;"
  "popal                        ;"
  "addl  $8, %esp               ;" // Pop error code and int vector
  "iret                         ;");

#define ISR_WITH_ERROR_CODE(int_vec, name)     \
  __asm__(                                     \
    ".global _" #name "         ;"             \
    "_" #name ":                ;"             \
    "pushl  " #int_vec "        ;"             \
    "jmp    isr_handler         ;"             \
  );

#define ISR_WITHOUT_ERROR_CODE(int_vec, name)  \
  __asm__(                                     \
    ".global _" #name "         ;"             \
    "_" #name ":                ;"             \
    "pushl  $0                  ;"             \
    "pushl  $" #int_vec "       ;"             \
    "jmp    isr_handler         ;"             \
  );

__asm__(
  ".global irq_handler          ;"
  "irq_handler:                 ;"
  "pushal                       ;"
  "pushl  %gs                   ;"
  "pushl  %fs                   ;"
  "pushl  %ds                   ;"
  "pushl  %es                   ;"
  "pushl  %ebp                  ;"
  "movl   %esp, %ebp            ;"
  "movw   $0x0010, %ax          ;"
  "movw   %ax, %ds              ;"
  "movw   %ax, %es              ;"
  "movw   %ax, %fs              ;"
  "movw   %ax, %gs              ;"
  "movl   52(%esp), %eax        ;" // IRQ number
  "call   *_irq_funcs(,%eax,4)  ;"
  "movb   $0x20, %al            ;"
  "cmpl   $0x78, 52(%esp)       ;"
  "jge    1f                    ;"
  "outb   %al, $0xa0            ;"
  "1:                           ;"
  "outb   %al, $0x20            ;"
  "leave                        ;"
  "popl  %es                    ;"
  "popl  %ds                    ;"
  "popl  %fs                    ;"
  "popl  %gs                    ;"
  "popal                        ;"
  "addl  $4, %esp               ;" // Pop IRQ number
  "iret                         ;");

#define IRQ_ISR(irq_num)                       \
  __asm__(                                     \
    ".global _irq_" #irq_num "_isr ;"          \
    "_irq_" #irq_num "_isr:        ;"          \
    "pushl  $" #irq_num "                ;"    \
    "jmp    irq_handler                 ;");

ISR_WITHOUT_ERROR_CODE(0xff, general_isr);
ISR_WITHOUT_ERROR_CODE(0x00, divide_error_isr);
ISR_WITHOUT_ERROR_CODE(0x01, debug_isr);
ISR_WITHOUT_ERROR_CODE(0x02, nmi_isr);
ISR_WITHOUT_ERROR_CODE(0x03, breakpoint_isr);
ISR_WITHOUT_ERROR_CODE(0x04, overflow_isr);
ISR_WITHOUT_ERROR_CODE(0x05, bound_range_isr);
ISR_WITHOUT_ERROR_CODE(0x06, invalid_opcode_isr);
ISR_WITHOUT_ERROR_CODE(0x07, no_coprocessor_isr);
ISR_WITH_ERROR_CODE(0x08, double_fault_isr);
ISR_WITH_ERROR_CODE(0x0a, invalid_tss_isr);
ISR_WITH_ERROR_CODE(0x0b, segment_not_present_isr);
ISR_WITH_ERROR_CODE(0x0c, stack_fault_isr);
ISR_WITH_ERROR_CODE(0x0d, general_protection_isr);
ISR_WITH_ERROR_CODE(0x0e, page_fault_isr);
ISR_WITHOUT_ERROR_CODE(0x10, math_fault_isr);
ISR_WITH_ERROR_CODE(0x11, align_check_isr);
ISR_WITHOUT_ERROR_CODE(0x12, machine_check_isr);
ISR_WITHOUT_ERROR_CODE(0x13, simd_fault_isr);

// ISRs for IRQs
IRQ_ISR(0);
IRQ_ISR(1);
IRQ_ISR(2);
IRQ_ISR(3);
IRQ_ISR(4);
IRQ_ISR(5);
IRQ_ISR(6);
IRQ_ISR(7);
IRQ_ISR(8);
IRQ_ISR(9);
IRQ_ISR(10);
IRQ_ISR(11);
IRQ_ISR(12);
IRQ_ISR(13);
IRQ_ISR(14);
IRQ_ISR(15);
