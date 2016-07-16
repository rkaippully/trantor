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
ASFLAGS := -O3 -Wall -march=i586

# Always use gcc as the compiler
CC := i586-elf-gcc

# Command line flags for the compiler
CFLAGS := -O3 -Wall -march=i586 -std=c99 -ffreestanding

# Always use ld for linking
LD := i586-elf-ld

# Command line flags for the linker
LIBDIR := $(dir $(shell which i586-elf-gcc))/../lib/gcc/i586-elf/$(shell i586-elf-gcc -dumpversion)
LDFLAGS := -nostartfiles -nodefaultlibs -nostdlib -static -m elf_i386 -T trantor.ld -Ttext=0xf0000000 -L$(LIBDIR) -lgcc

# How to compile assembly source code
%.o: %.S
	$(AS) $(ASFLAGS) -c -o $*.o $*.S

# How to compile C source code
%.o: %.c
	$(CC) $(CFLAGS) -c -o $*.o $*.c

# The kernel is compiled to this file
IMAGE=trantor

# Kernel is composed of these object files
OBJECTS := boot.o main.o mm.o pic.o isr.o interrupts.o timer.o kmalloc.o


# The default Make target will compile the kernel
$(IMAGE): $(OBJECTS)
	$(LD) $(LDFLAGS) -Map=$(IMAGE).map -o $@ $^

# Emulate the operaing system with qemu
emulate: $(IMAGE)
	qemu-system-i386 -cpu pentium -m 4 -kernel $(IMAGE) -monitor stdio -no-shutdown

clean:
	$(RM) $(IMAGE) $(IMAGE).map $(OBJECTS)
