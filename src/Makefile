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

TEST_IMAGE := test/trantor.img

all:
	@$(MAKE) --no-print-directory -C lib
	@$(MAKE) --no-print-directory -C kernel
	@$(MAKE) --no-print-directory -C kinit
	@$(MAKE) --no-print-directory -C mm

$(TEST_IMAGE): all
	@test/mkhdimg $@

qemu: $(TEST_IMAGE)
	@qemu-system-i386 -cpu 486 -m 2 -drive file=$(TEST_IMAGE),index=0,media=disk,format=raw -monitor stdio -no-shutdown -debugcon file:/dev/stdout -name 'Trantor OS'

bochs: $(TEST_IMAGE)
	@bochs -f test/bochsrc.hd

clean:
	@$(MAKE) --no-print-directory -C lib clean
	@$(MAKE) --no-print-directory -C kernel clean
	@$(MAKE) --no-print-directory -C kinit clean
	@$(MAKE) --no-print-directory -C mm clean
	@$(RM) $(TEST_IMAGE)

.PHONY: all qemu bochs clean
