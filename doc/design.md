# Design of Trantor OS

## Introduction

Trantor is a 32-bit operating system compatible with MS-DOS 6.22 and DPMI
specification 1.0. Trantor runs on IBM PC compatible systems with the following
hardware.

* 80386 or compatible CPU
* At least 2 MB RAM
* VGA/SVGA compatible video card
* PS/2 keyboard
* PS/2 mouse
* Floppy disk
* IDE HD/CD-ROM

Trantor can run more than one DOS environment and provide multitasking. Each DOS
environment is called a DOS console. You can switch between consoles with
Alt+Tab. You can launch a new console with Ctrl+Esc. There is only one console
created at boot. But more can be launched with Ctrl+Esc.

## Architecture

Trantor is a 32-bit multitasking operating system running on IA-32 compatible
CPUs. It has a monolithic kernel with some limited support for loading driver
modules.
