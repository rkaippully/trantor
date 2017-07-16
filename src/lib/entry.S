/*
	Trantor Operating System

	Copyright (C) 2017 Raghu Kaippully

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.

  This Source Code Form is "Incompatible With Secondary Licenses", as
	defined by the Mozilla Public License, v. 2.0.
*/

#define USER_STACK_SIZE 4096

  .arch i486

  .section ".text.startup"
	/* Entry point of applications */
  .global _start
_start:
  /* Set up a stack */
	movl   $(user_stack + USER_STACK_SIZE), %esp

  call   main

  /* TODO: handle exit */

forever:
  hlt
  jmp  forever



  .bss
  .balign 0x1000
user_stack:
	.fill USER_STACK_SIZE, 1, 0
