/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#include "asm.h"
#include "drivers.h"
#include "printf.h"
#include "interrupts.h"

static const uint16_t DATA_PORT = 0x60, STATUS_PORT = 0x64, COMMAND_PORT = 0x64;

static uint8_t receive()
{
  while (!(inb(STATUS_PORT) & 1));
  return inb(DATA_PORT);
}

static void send(uint8_t b)
{
  while (inb(STATUS_PORT) & 2);
  outb(DATA_PORT, b);
}

static void keyboard_irq()
{
  kdebug("keyboard: ");
  do {
    uint8_t b = receive();
    kdebug("%02x ", b);
  } while (inb(STATUS_PORT) & 1);
  kdebug("\n");
}

static void init_failed()
{
  cprintf("Keyboard initialization failed. System halted.");
  halt();
}

/*
  Initialize PS/2 keyboard.
*/
static void keyboard_init()
{
  // Reset device
  send(0xff);
  if (receive() != 0xfa)
    init_failed();
  if (receive() != 0xaa)
    init_failed();

  // Set NumLock LED
  send(0xed);
  send(0x02);
  if (receive() != 0xfa)
    init_failed();

  // Read a few times to clear the buffer
  io_wait();
  for (int i = 0; i < 10; i++) {
    inb(DATA_PORT);
    io_wait();
  }

  irq_funcs[9] = keyboard_irq;

  kdebug("Keyboard initialized.\n");
}

driver_t keyboard_driver = {
  .init = keyboard_init,
  .shutdown = 0
};
