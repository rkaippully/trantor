/*
	Trantor Operating System

	Copyright (C) 2017 Raghu Kaippully

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.

  This Source Code Form is "Incompatible With Secondary Licenses", as
	defined by the Mozilla Public License, v. 2.0.
*/

#include "kernel/memory.h"

  .arch i486

  .text

	/* Entry point of kinit */
  .global main
main:
  /* Set up a stack */
	movl   $(kernel_stack + KERNEL_STACK_SIZE), %esp
	subl   $_kernel_start, %esp
	addl   $_kinit_end, %esp

  pushl  %ebx
  pushl  %eax
  call   kmain

check_error:
  cmpl   $0, %eax
  je     forever

	/* Convert %al to BCD */
  pushl  %eax
  call   to_bcd
  addl   $4, %esp

  /* Display it */
  movl   $0xb8000, %edi
  movl   $0x47004700, (%edi)
  movb   %ah, (%edi)
  movb   %al, 2(%edi)

forever:
  hlt
  jmp  forever

  .data

test_data:
  .byte 3

  .bss

  .global test_bss
test_bss:
  .byte 0
