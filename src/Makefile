#
# Trantor Operating System
#
# Copyright (C) 2017 Raghu Kaippully
#

#
# Makefile for Trantor OS
#

GCC_PREFIX := i586-pc-msdosdjgpp

# Always use gcc as assembler
AS := $(GCC_PREFIX)-gcc

# Command line flags for assembler
ASFLAGS := -O3 -Wall -I boot

# Always use gcc as the compiler
CC := $(GCC_PREFIX)-gcc

# Command line flags for the compiler
CFLAGS := -O3 -Wall -march=i386 -std=c11 -ffreestanding -Iinclude

# Always use ld for linking
LD := $(GCC_PREFIX)-ld
LINKER_SCRIPT := trantor.ld

# Command line flags for the linker
LIBDIR := $(dir $(shell which $(CC)))../lib/gcc/$(GCC_PREFIX)/$(shell $(CC) -dumpversion)
LDFLAGS := -nostartfiles -nodefaultlibs -nostdlib -static -L$(LIBDIR) -lgcc

# How to compile assembly source code
%.o: %.S
	$(AS) $(ASFLAGS) -c -o $*.o $*.S

# How to compile C source code
%.o: %.c
	$(CC) $(CFLAGS) -c -o $*.o $*.c

# The kernel is compiled to this file
KERNEL := trantor
KERNEL_IMG := $(KERNEL).sys

# Boot sector / MBR images
BOOT_IMGS   := boot/bootchs.img boot/bootlba.img boot/mbr.img
PRELUDE_IMG := boot/prelude.img

# Kernel is composed of these object files
KOBJECTS := kernel/main.o

boot/%.img: boot/%.o
	$(LD) $(LDFLAGS) -T trantor-boot.ld -o $@ $^

.phony: clean qemu bochs

$(KERNEL_IMG): $(PRELUDE_IMG) $(KERNEL)
	cat $^ > $@

$(KERNEL): $(KOBJECTS)
	$(LD) $(LDFLAGS) -T $(LINKER_SCRIPT) -Map=$(KERNEL).map -o $@ $^

# hard disk image to emulate the operaing system
HD_IMG := trantor.hd

$(HD_IMG): $(KERNEL) $(BOOT_IMGS)
	tools/mkhdimg $^ $@

qemu: $(HD_IMG)
	qemu-system-i386 -cpu 386 -m 2 -hda $(HD_IMG) -monitor stdio -no-shutdown

bochs: CFLAGS += -DUSE_BOCHS_E9
bochs: $(HD_IMG)
	bochs -f bochsrc

clean:
	$(RM) $(KERNEL) $(KERNEL).map $(KERNEL_IMG) $(BOOT_IMGS) $(PRELUDE_IMG) $(KOBJECTS) $(HD_IMG)
