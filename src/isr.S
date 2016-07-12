/*
    Trantor Operating System
    Copyright (C) 2014 Raghu Kaippully

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
    We need to deal with two types of interrupts. Processor generated interrupts and I/O
    device gemenrated IRQs. This file defines interrupt service routines in assembly.
    These ISRs does some initial set up and then invoke C functions defined in
    interrupts.c.
*/

#define KERNEL_DS 0x10

/* A macro to create an ISR */
.macro setup_isr vector, name, has_error_code=0
    .global \name\()_isr_wrapper
    \name\()_isr_wrapper:
        /*
            Some exceptions do not have an error code, we push a dummy code (0) in that
            case
        */
        .if \has_error_code == 0
        pushl   $0
        .endif
        /* Save all registers */
        pushal
        /* Save the return address in %ebx */
        movl    $1f, %ebx
        jmp     _save_regs
1:
        /* Interrupt vector number is also pushed */
        pushl   $\vector

        /* Call the C ISR function */
        call    \name\()_isr
        jmp     _leave_isr
.endm

/* Another macro to create an ISR for IRQs */
.macro setup_irq_isr vector, name, c_func_name=irq_isr
    .global \name\()_isr_wrapper
    \name\()_isr_wrapper:
        /*
            Push a dummy error code to be consistent with non-IRQ ISRs. This will keep the
            stack size the same for IRQs and interrupts and thus help to simplify code in
            _leave_isr.
        */
        pushl   $0
        /* Save all registers */
        pushal
        /* Save the return address in %ebx */
        movl    $1f, %ebx
        jmp     _save_regs
1:
        /* IRQ number is also pushed */
        pushl   $(\vector - 0x80)

        /* Call the C ISR function */
        call    \c_func_name
        jmp     _leave_irq_isr
.endm


.text

/*
    Called at the begining of ISR to save all registers in the stack and switch the data
    segments.
*/
_save_regs:
        pushl   %ds
        pushl   %es
        pushl   %fs
        pushl   %gs
        /* Switch to kernel data segment */
        movw    $KERNEL_DS, %ax
        movw    %ax, %ds
        movw    %ax, %es
        movw    %ax, %fs
        movw    %ax, %gs
        /* Go back to the ISR */
        jmp     *%ebx

/*
    We should send an end of interrupt command to the PIC before leaving an IRQ ISR
*/
_leave_irq_isr:
        movb    $0x20, %al
        /* The IRQ number is stored at (%esp), check if it is master/slave IRQ */
        cmpl    $8, (%esp)
        jl      1f
        /* Send EOI to slave PIC */
        outb    %al, $0xa0
1:
        /* Send EOI to master PIC */
        outb    %al, $0x20
        /* Now we just follow the logic in _leave_isr */

/*
    Restore registers and return from ISR
*/
_leave_isr:
        /* Pop the interrupt vector/IRQ number from stack */
        addl    $4, %esp
        /* Restore registers */
        popl    %gs
        popl    %fs
        popl    %es
        popl    %ds
        popal
        /* Pop the error code from stack */
        addl    $4, %esp
        iret

// An ISR that does not do anything
.global nop_isr
nop_isr:
    iret

setup_isr 0, divide_error
setup_isr 2, nmi
setup_isr 3, breakpoint
setup_isr 4, overflow
setup_isr 5, bound_range
setup_isr 6, invalid_opcode
setup_isr 7, device_unavailable
setup_isr 8, double_fault, 1
setup_isr 10, invalid_tss, 1
setup_isr 11, segment_not_present, 1
setup_isr 12, stack_fault, 1
setup_isr 13, gp_fault, 1
setup_isr 14, page_fault, 1
setup_isr 16, fpu_error
setup_isr 17, alignment_check, 0
setup_isr 18, machine_check

setup_irq_isr 0x80, irq_0, timer_isr
setup_irq_isr 0x81, irq_1
setup_irq_isr 0x82, irq_2
setup_irq_isr 0x83, irq_3
setup_irq_isr 0x84, irq_4
setup_irq_isr 0x85, irq_5
setup_irq_isr 0x86, irq_6
setup_irq_isr 0x87, irq_7
setup_irq_isr 0x88, irq_8
setup_irq_isr 0x89, irq_9
setup_irq_isr 0x8a, irq_a
setup_irq_isr 0x8b, irq_b
setup_irq_isr 0x8c, irq_c
setup_irq_isr 0x8d, irq_d
setup_irq_isr 0x8e, irq_e
setup_irq_isr 0x8f, irq_f
