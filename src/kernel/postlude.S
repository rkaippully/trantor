/*
	Trantor Operating System

	Copyright (C) 2016 Raghu Kaippully
*/

/*
  This file contains sections that are tacked at the end of the kernel but not
	part of it. These will be relocated by boot.S.
*/

.section .kernel_postlude, "aw", @nobits

/*
  A 4KB sized kernel stack.
*/
.align 4096
.global kernel_stack_start, kernel_stack_end
kernel_stack_start:
.fill 1024, 4, 0
kernel_stack_end:

/*
  Page table used by the physical memory manager stacks.

	We can have a maximum of 1M pages and each page will take 4 bytes in the
	stack. Hence, we need a maximum of 4MB for both stacks together. This means
	we need to allocate only one page for the page table.
*/
.align 4096
.global pmm_stack_page_table
pmm_stack_page_table:
.fill 1024, 4, 0
