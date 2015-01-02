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

#include "ints.h"
#include "asm.h"

/*
    Intel had reserved interrupts 0-31 in 8086 but IBM decided to use them anyway in the
    original PC. And we are still stuck with that design decision in the current PCs. But
    the CPU uses some of those interrupts in protected mode and will result in a conflict.
    So we remap those interrupt vectors using the programmable interrupt controller (PIC).
    IRQs 0-15 are mapped to interrupt vectors 0x80-0x8f respectively.

    There are two PICs - a master and a slave. 
*/

/* 
    I/O ports for master and slave PIC. The command posrt is used to send commands and get
    status. The data port is for transferring data and to get access interrupt masks.
*/
static const uint16_t MASTER_PIC_COMMAND_PORT = 0x20;
static const uint16_t MASTER_PIC_DATA_PORT    = 0x21;
static const uint16_t SLAVE_PIC_COMMAND_PORT  = 0xa0;
static const uint16_t SLAVE_PIC_DATA_PORT     = 0xa1;

/* Initialization Control Words (ICWs) */

/*
    ICW1
    Bit 0: expect ICW4 as part of initialization
    Bit 4: set if PIC needs to be initialized
*/
static const uint8_t ICW1_VAL = 0x11;

/* ICW2: IRQ starting vectors for both master and slave */
static const uint8_t IRQ_0 = 0x80;
static const uint8_t IRQ_8 = 0x88;

/* 
    ICW3 is used to let PICs know how they are connected to each other. The slave is
    connected to IR2 line on the master. We set the corresponding bit (bit 2) in the
    master PIC - i.e. value 0x04. We also set this value in binary form (010 = 0x02) on
    the slave PIC.
*/
static const uint8_t ICW3_MASTER_VAL = 0x04;
static const uint8_t ICW3_SLAVE_VAL  = 0x02;

/*
    ICW4: we are only interested in bit 0 that puts PIC in x86 mode
*/
static const uint8_t ICW4_VAL = 1;

void pic_init()
{
    // ICW1: Start initialization sequence
    out_byte(MASTER_PIC_COMMAND_PORT, ICW1_VAL);
    out_byte(SLAVE_PIC_COMMAND_PORT, ICW1_VAL);

    // ICW2: Send IRQ vectors to data ports
    out_byte(MASTER_PIC_DATA_PORT, IRQ_0);
    out_byte(SLAVE_PIC_DATA_PORT, IRQ_8);

    // ICW3: set the IR line to connect slave and master
    out_byte(MASTER_PIC_DATA_PORT, ICW3_MASTER_VAL);
    out_byte(SLAVE_PIC_DATA_PORT, ICW3_SLAVE_VAL);

    // ICW4: set the PIC to x86 mode
    out_byte(MASTER_PIC_DATA_PORT, ICW4_VAL);
    out_byte(SLAVE_PIC_DATA_PORT, ICW4_VAL);

    // Writing 0 to interrupt mask register will enable all interrupts
    out_byte(MASTER_PIC_DATA_PORT, 0);
    out_byte(SLAVE_PIC_DATA_PORT, 0);
}

/* vim: set expandtab ai nu ts=4 tw=90: */
