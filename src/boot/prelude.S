/*
	Trantor Operating System

	Copyright (C) 2017 Raghu Kaippully
*/

/*
  This file contains code tacked at the start of trantor.sys. It helps load the
  kernel COFF format and set up protected mode.
*/

.arch i186
.code16
.text

  movw  $0x0941, %ax
  movw  $0x00fc, %bx
  movw  $10, %cx
  int   $0x10
1:
  hlt
  jmp   1b
