/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

/*
  The boot console is available to display messages to user during booting.
  The console should not be used after initialization is complete.
*/

#include "stdint.h"
#include "asm.h"
#include "stdarg.h"
#include "printf.h"
#include "version.h"

static uint8_t* VMEMBASE = (uint8_t*)0xc00b8000;
static int cursor_pos;

static void update_cursor()
{
  /* cursor LOW port to vga INDEX register */
  outb(0x3d4, 0x0f);
  outb(0x3d5, (uint8_t)(cursor_pos & 0xff));
  /* cursor HIGH port to vga INDEX register */
  outb(0x3d4, 0x0e);
  outb(0x3d5, (uint8_t)((cursor_pos >> 8) & 0xff));
}

void init_console()
{
  char header[80];
  sprintf(header, "Trantor OS %d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
  int len = 0;
  while (len < 80 && header[len])
    len++;

  uint16_t* p = (uint16_t*)VMEMBASE;
  int i = 0;

  for (; i < (80 - len)/2; i++)
    p[i] = 0x1f20;
  for (int j = 0; j < len; i++, j++)
    p[i] = 0x1f00 | header[j];
  for (; i < 80; i++)
    p[i] = 0x1f20;

  for (; i < 80*25; i++)
    p[i] = 0x0720;

  cursor_pos = 80;
  update_cursor();
}

static void scroll_up_if_needed()
{
  if (cursor_pos == 80*25) {
    for (int i = 80; i < 80*24*2; i++)
      VMEMBASE[i] = VMEMBASE[i+80*2];
    uint16_t* p = ((uint16_t*)VMEMBASE) + 80*24;
    for (int i = 0; i < 80; i++)
      p[i] = 0x0720;
    cursor_pos = 80*24;
  }
}

static void write_char(unsigned char c)
{
  scroll_up_if_needed();
  VMEMBASE[cursor_pos*2] = c;
  cursor_pos++;
}

void cputc(unsigned char c)
{
  int col = cursor_pos % 160;
  switch (c) {
  case '\t':
    do {
      write_char(' ');
      col = cursor_pos % 80;
    } while ((col % 16) != 0);
    break;
  case '\r':
    while (col != 0) {
      cursor_pos--;
      col = cursor_pos % 80;
    }
    break;
  case '\n':
    do {
      cursor_pos++;
      col = cursor_pos % 80;
    } while (col != 0);
    scroll_up_if_needed();
    break;
  default:
    write_char(c);
    break;
  }
  update_cursor();
}

void cprintf(const char* format, ...)
{
  char buf[MAX_BUF_SIZE];
  va_list ap;

  va_start(ap, format);
  vsprintf(buf, format, ap);
  va_end(ap);

  for(int i = 0; i < MAX_BUF_SIZE && buf[i]; i++)
  cputc(buf[i]);
}
