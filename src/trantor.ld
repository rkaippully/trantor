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
    Trantor linker script

    A linker script gives us a lot of control over the generated kernel image, such as
    the load address, output format, layout of sections in the image etc.
*/

/* Use 32 bit format */
OUTPUT_FORMAT("elf32-i386")

/* The symbol at which execution starts */
ENTRY(_start)

SECTIONS {

    /* The image is loaded at 1 MB in RAM */
    .text : AT(0x00100000) {
        /* Put the multiboot section at the begining so that it is within the first 8KB */
        *(.multiboot)

        *(.text)
        *(.text.*)
    }

    /* Data section follows text section */
    .data ADDR(.text) + SIZEOF(.text) : {
        *(.data)
        *(.data2)
        *(.rodata)
        *(.rodata.*)
    }

    /* BSS follows the data section */
    .bss ADDR(.data) + SIZEOF(.data) : {
        *(.bss)
        *(COMMON)
    
        /* Physical memory bitmap is stored at the end of kernel */
        *(.membitmap)

        kernel_end = ALIGN(4096);
    }
    
    /* Stuff we don't want in the kernel */
    /DISCARD/ : {
        *(.eh_frame)
        *(.comment)
    }
} 

/* vim: set expandtab ai ts=4 tw=90: */
