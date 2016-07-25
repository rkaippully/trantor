#
# Trantor Operating System
#
# Copyright (C) 2014 Raghu Kaippully
#

#
# Makefile for Trantor OS
#

# Always use gcc as assembler
AS := i586-elf-gcc

# Command line flags for assembler
ASFLAGS := -O3 -Wall -march=pentium-mmx

# Always use gcc as the compiler
CC := i586-elf-gcc

# Command line flags for the compiler
CFLAGS := -O3 -Wall -march=pentium-mmx -std=c11 -ffreestanding -Iinclude

# Always use ld for linking
LD := i586-elf-ld
LINKER_SCRIPT := trantor.ld

# Command line flags for the linker
LIBDIR := $(dir $(shell which i586-elf-gcc))/../lib/gcc/i586-elf/$(shell i586-elf-gcc -dumpversion)
LDFLAGS := -nostartfiles -nodefaultlibs -nostdlib -static -m elf_i386 -T $(LINKER_SCRIPT) -L$(LIBDIR) -lgcc

# How to compile assembly source code
%.o: %.S
	$(AS) $(ASFLAGS) -c -o $*.o $*.S

# How to compile C source code
%.o: %.c
	$(CC) $(CFLAGS) -c -o $*.o $*.c

# The kernel is compiled to this file
IMAGE=trantor

# Kernel is composed of these object files
KINIT_OBJECTS := kinit/boot.o kinit/main.o kinit/mminit.o
KERNEL_OBJECTS := kernel/mmbitmap.o kernel/mm.o kernel/pic.o kernel/isr.o kernel/interrupts.o kernel/timer.o kernel/kmalloc.o
OBJECTS := $(KINIT_OBJECTS) $(KERNEL_OBJECTS)


# The default Make target will compile the kernel
$(IMAGE): $(OBJECTS) $(LINKER_SCRIPT)
	$(LD) $(LDFLAGS) -Map=$(IMAGE).map -o $@ $^

# Emulate the operaing system
HD_IMAGE=$(IMAGE).hd

$(HD_IMAGE): $(IMAGE)
	./mkhdimg $^ $@

qemu: $(HD_IMAGE)
	qemu-system-i386 -cpu pentium -m 32 -hda $(HD_IMAGE) -monitor stdio -no-shutdown

bochs: $(HD_IMAGE)
	bochs -f bochsrc

clean:
	$(RM) $(IMAGE) $(IMAGE).map $(OBJECTS) $(HD_IMAGE)
