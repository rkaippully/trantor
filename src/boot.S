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
    Multiboot header

    Multiboot header should be located within the first 8KB of the image and must be
    32-bit aligned. Since our image is very small this will always be under 8KB.
*/
.section .multiboot
.align 4
multiboot_header:
    .long   0x1badb002      # multiboot header magic
    .long   0               # flags bitmap - we don't use any of this for now
    .long   0xe4524ffe      # checksum - when added to magic and flags this must give zero
    .long   0, 0, 0, 0, 0   # the address fields of multiboot header are unused
    .long   1               # video mode - set standard EGA 80x25 text mode
    .long   80              # video mode - 80 columns
    .long   25              # video mode - 25 rows
    .long   0               # video mode depth - should be 0 for a text mode


/* The text section starts here */
.text

/* Kernel starts execution here */
.global _start
_start:
    /*
        Display the character 'A' at the top left of the screen. The video memory is
        located at the address 0xb8000. So we write 'A' to that address. The next byte at
        0xb8001 is the color byte, its layout is:

        Bit 7 6 5 4 3 2 1 0
            | | | | | | | |
            | | | | | ^ ^ ^---foreground color in RGB
            | | | | ^---------foreground color bright bit
            | ^ ^ ^-----------background color in RGB
            ^-----------------back colour bright bit OR enables blinking Text

        A value of 0xf4 will display the letter in red color on a bright white background.
    */
    movb $'A, 0xb8000
    movb $0xf4, 0xb8001

    /* Halt the processor after the character is displayed */
    hlt


/* vim: set expandtab ai ts=4 tw=90: */
