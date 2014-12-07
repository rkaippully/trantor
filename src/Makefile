# 
# Trantor Operating System
# Copyright (C) 2014 Raghu Kaippully
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

#
# Makefile for Trantor OS
#

# Always use gcc as assembler
AS := gcc

# Command line flags for assembler
ASFLAGS := -O3 -Wall -m32 -march=i586

# Always use ld for linking
LD := ld

# Command line flags for the linker
LDFLAGS := -nostartfiles -nodefaultlibs -nostdlib -static --strip-all -m elf_i386 -T trantor.ld

# How to compile assembly source code
%.o: %.S
	$(AS) $(ASFLAGS) -c -o $*.o $*.S


# The kernel is compiled to this file
IMAGE=trantor.img

# Kernel is composed of these object files
OBJECTS := boot.o


# The default Make target will compile the kernel
$(IMAGE): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

# Emulate the operaing system with qemu
emulate: $(IMAGE)
	qemu-system-i386 -cpu pentium -m 4 -kernel $(IMAGE)

clean:
	$(RM) $(IMAGE) $(OBJECTS)
