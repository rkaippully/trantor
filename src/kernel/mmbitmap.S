/*
	Trantor Operating System

	Copyright (C) 2016 Raghu Kaippully
*/


.section .membitmap, "aw", @nobits

.global pmm_bitmap_array
pmm_bitmap_array:
  .fill 128 * 1024, 1, 0
