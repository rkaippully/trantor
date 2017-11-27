# Design of Trantor OS

## Introduction

Trantor is a 32-bit operating system compatible with MS-DOS 6.22 and DPMI
specification 1.0. Trantor runs on IBM PC compatible systems with the following
hardware.

* 80486 or compatible CPU
* At least 2 MB RAM
* VGA/SVGA compatible video card
* PS/2 keyboard
* IDE HDD

And optionally:

* PS/2 mouse
* Floppy disk
* IDE CD-ROM

## Architecture

Trantor is a 32-bit multitasking operating system running on IA-32 compatible
CPUs. It has a monolithic kernel with some limited support for loading driver
modules.

The focus is on getting a DOS clone that can run older games.
