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
CFLAGS := -O3 -Wall -march=i386 -std=c11 -ffreestanding -nostdinc -Iinclude -DUSE_E9_DEBUG

# Always use ld for linking
LD := $(GCC_PREFIX)-ld
LINKER_SCRIPT := trantor.ld

# Command line flags for the linker
LIBDIR := $(dir $(shell which $(CC)))../lib/gcc/$(GCC_PREFIX)/$(shell $(CC) -dumpversion)
LDFLAGS := -nostartfiles -nodefaultlibs -nostdlib -static -L$(LIBDIR) -lgcc -s

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
KOBJECTS := kernel/boot/main.o kernel/boot/console.o kernel/boot/meminit.o kernel/pmm.o kernel/kprintf.o

boot/%.img: boot/%.o
	$(LD) $(LDFLAGS) -T trantor-boot.ld -o $@ $^

.phony: clean qemu bochs qemu-fd

$(KERNEL_IMG): $(PRELUDE_IMG) $(KERNEL)
	cat $^ > $@

$(KERNEL): $(KOBJECTS)
	$(LD) $(LDFLAGS) -T $(LINKER_SCRIPT) -Map=$(KERNEL).map -o $@ $^



#
#	Testing/Emulation
#

T_CC := gcc
T_CFLAGS := -O3 -Wall
T_LD := gcc
T_LDFLAGS :=
T_MKBOOT := tools/mkboot

# disk images to emulate/test the operaing system
FD_IMG := trantor.fd
HD_IMG := trantor.hd

$(FD_IMG): $(KERNEL_IMG) $(BOOT_IMGS) $(T_MKBOOT)
	tools/mkfdimg $(KERNEL_IMG) $(FD_IMG)
	$(T_MKBOOT) $(FD_IMG) $(BOOT_IMGS)

$(HD_IMG): $(KERNEL_IMG) $(BOOT_IMGS) $(T_MKBOOT)
	tools/mkhdimg $(KERNEL_IMG) $(HD_IMG)
	$(T_MKBOOT) $(HD_IMG) $(BOOT_IMGS)

tools/%: tools/%.o
	$(T_LD) $(T_LDFLAGS) -o $@ $^

tools/%.o: tools/%.c
	$(T_CC) $(T_CFLAGS) -c -o $@ $^

qemu: $(HD_IMG)
	qemu-system-i386 -cpu 486 -m 2 -drive file=$(HD_IMG),index=0,media=disk,format=raw -monitor stdio -no-shutdown -debugcon file:/dev/stdout

qemu-fd: CFLAGS+=-DUSE_E9_DEBUG
qemu-fd: $(FD_IMG)
	qemu-system-i386 -cpu 486 -m 2 -drive file=$(FD_IMG),index=0,if=floppy,format=raw -monitor stdio -no-shutdown -debugcon file:/dev/stdout

bochs: $(HD_IMG)
	bochs -f bochsrc.hd

bochs-fd: CFLAGS+=-DUSE_E9_DEBUG
bochs-fd: $(FD_IMG)
	bochs -f bochsrc.fd

clean:
	$(RM) $(KERNEL) $(KERNEL).map $(KERNEL_IMG) $(BOOT_IMGS) $(PRELUDE_IMG) $(KOBJECTS) $(FD_IMG) $(HD_IMG) $(T_MKBOOT)
