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
# Makefile for Trantor mm process
#

include ../build.mk

IMAGE     := mm
IMAGE_MAP := $(basename $(IMAGE)).map
OBJECTS   := ../lib/libtrantor.a entry.o main.o

all: $(IMAGE)

$(IMAGE): $(OBJECTS)
	@$(LD) $(LDFLAGS) -T ../ldscripts/bootmodule.ld -Map=$(IMAGE_MAP) -o $@ $^

clean:
	@$(RM) $(OBJECTS) $(IMAGE) $(IMAGE_MAP)

.PHONY: all clean
