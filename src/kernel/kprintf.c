/*
  Trantor Operating System

  Copyright (C) 2017 Raghu Kaippully
*/

#include "stdint.h"
#include "stdarg.h"
#include "asm.h"
#include "printf.h"

static inline int isdigit(char c)
{
  return c >= '0' && c <= '9';
}

/*
    A very simple implementation. Use with care.
*/
void vsprintf(char* buf, const char* fmt, va_list args)
{
  uint32_t unumber, i, idx = 0;
  int32_t number;
  char* s;
  static const char* hexchars = "0123456789abcdef";
  static const char* decchars = "0123456789";
  char digits[20];
  uint32_t do_pad = 0;

  for (; *fmt && idx < MAX_BUF_SIZE-1; fmt++) {
    if (*fmt != '%' && do_pad == 0) {
      buf[idx++] = *fmt;
      continue;
    }

    switch (*++fmt) {
    case '%':
      buf[idx++] = '%';
      do_pad = 0;
      break;
    case 'c':
      buf[idx++] = (unsigned char)va_arg(args, unsigned char);
      do_pad = 0;
      break;
    case 's':
      s = va_arg(args, char*);
      while (*s && idx < MAX_BUF_SIZE-1)
        buf[idx++] = *s++;
      do_pad = 0;
      break;
    case '0':
      for (; isdigit(*fmt); fmt++)
        do_pad = do_pad * 10 + *fmt - '0';
      fmt -= 2;
      break;
    case 'p':
    case 'x':
      unumber = va_arg(args, uint32_t);
      i = 19;
      do {
        digits[i--] = hexchars[unumber % 16];
        unumber /= 16;
      } while (unumber > 0);
      while (i > 19 - do_pad)
        digits[i--] = '0';
      i++;
      while (i < 20 && idx < MAX_BUF_SIZE-1)
        buf[idx++] = digits[i++];
      do_pad = 0;
      break;
    case 'd':
      number = va_arg(args, int32_t);
      i = 19;
      do {
        digits[i--] = decchars[number % 10];
        number /= 10;
      } while (number > 0);
      while (i > 19 - do_pad)
        digits[i--] = '0';
      i++;
      if (number < 0)
        buf[idx++] = '-';
      while (i < 20 && idx < MAX_BUF_SIZE-1)
        buf[idx++] = digits[i++];
      do_pad = 0;
      break;
    }
  }
  buf[idx] = '\0';
}

void sprintf(char* buf, const char* format, ...)
{
  va_list ap;

  va_start(ap, format);
  vsprintf(buf, format, ap);
  va_end(ap);
}

#ifdef USE_E9_DEBUG
/*
    Writes to port 0xe9 - for bochs/qemu debug hack
*/
void kdebug(const char* format, ...)
{
  char buf[MAX_BUF_SIZE];
  va_list ap;

  va_start(ap, format);
  vsprintf(buf, format, ap);
  va_end(ap);

  for(int i = 0; i < MAX_BUF_SIZE && buf[i]; i++)
    outb(0xe9, buf[i]);
}
#endif
