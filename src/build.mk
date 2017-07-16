#
# Trantor Operating System
#
# Copyright (C) 2017 Raghu Kaippully
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This Source Code Form is "Incompatible With Secondary Licenses", as
# defined by the Mozilla Public License, v. 2.0.
#

#
# Common definitions for Makefiles
#

GCC_PREFIX := i486-pc-elf

AS      := $(GCC_PREFIX)-gcc
ASFLAGS := -Wall -c -I../include
CC      := $(GCC_PREFIX)-gcc
CFLAGS  := -O3 -Wall -march=i486 -std=c11 -pedantic -ffreestanding -nostdinc -I../include
LD      := $(GCC_PREFIX)-ld
LIBDIR  := $(dir $(shell which $(CC)))../lib/gcc/$(GCC_PREFIX)/$(shell $(CC) -dumpversion)
LDFLAGS := -nostartfiles -nodefaultlibs -nostdlib -static -L$(LIBDIR) -lgcc -s
ARFLAGS := r

KOBJECTS := process.o memory.o

%.o: %.c
	@echo Compiling $^
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $^

%.o: %.S
	@echo Assembling $^
	@$(AS) $(ASFLAGS) -c -o $@ $^
