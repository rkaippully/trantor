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
# Makefile for Trantor kinit process
#

include ../build.mk

IMAGE     := kinit
IMAGE_MAP := $(basename $(IMAGE)).map
OBJECTS   := entry.o multiboot.o main.o initrd.o memory.o util.o ../lib/libtrantor.a

all: $(IMAGE)

$(IMAGE): $(OBJECTS) $(foreach obj, $(KOBJECTS), ../kernel/$(obj))
	@$(LD) $(LDFLAGS) -T ../ldscripts/kinit.ld -Map=$(IMAGE_MAP) -o $@ $^

clean:
	@$(RM) $(OBJECTS) $(IMAGE) $(IMAGE_MAP)

.PHONY: all clean
