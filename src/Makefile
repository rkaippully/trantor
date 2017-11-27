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
# Makefile for Trantor OS
#

GCC_PREFIX := i486-pc-elf

AS        := $(GCC_PREFIX)-gcc
ASFLAGS   := -Wall -c -I./include
CC        := $(GCC_PREFIX)-gcc
CFLAGS    := -O3 -Wall -march=i486 -std=c11 -pedantic -ffreestanding -nostdinc -I./include
LD        := $(GCC_PREFIX)-ld
LIBDIR    := $(dir $(shell which $(CC)))../lib/gcc/$(GCC_PREFIX)/$(shell $(CC) -dumpversion)
LDFLAGS   := -nostartfiles -nodefaultlibs -nostdlib -static -L$(LIBDIR) -lgcc -s
RUSTC     := rustc
RUSTFLAGS := --target i586-unknown-linux-gnu -C target-cpu=i486 -O

KERNEL     := kernel.sys
KERNEL_MAP := $(basename $(KERNEL)).map
OBJECTS    := entry.o data.o lib.o

all: $(KERNEL)

%.o: %.c
	@echo Compiling $^
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $^

%.o: %.S
	@echo Assembling $^
	@$(AS) $(ASFLAGS) -c -o $@ $^

lib.o: *.rs
	@echo Compiling $@
	@$(RUSTC) $(RUSTFLAGS) --emit=obj -o $@ $(basename $@).rs

$(KERNEL): $(OBJECTS)
	@echo Linking $@
	@$(LD) $(LDFLAGS) -T kernel.ld -Map=$(KERNEL_MAP) -o $@ $^

TEST_IMAGE := test/trantor.img

$(TEST_IMAGE): $(KERNEL)
	@test/mkhdimg $@

qemu: $(TEST_IMAGE)
	@qemu-system-i386 -cpu 486 -m 2 -drive file=$(TEST_IMAGE),index=0,media=disk,format=raw -monitor stdio -no-shutdown -debugcon file:/dev/stdout -name 'Trantor OS'

bochs: $(TEST_IMAGE)
	@bochs -f test/bochsrc.hd

clean:
	@$(RM) $(OBJECTS) $(KERNEL) $(KERNEL_MAP) $(TEST_IMAGE)

.PHONY: all qemu bochs clean
