# #########################################################
# this makefile allows building and cleaning the toggle demo
#
# Copyright (c) 2011 Steve Denson
# MIT Licensed
# #########################################################

# Propeller Demo include
PROPLIB = ../../common

CFLAGS = -Os -mlmm

#
# objects for this program
#

NAME = toggle
OBJS = toggle.o toggle_firmware.o

include $(PROPLIB)/lmmdemo.mk

#%.dat: $(SPINDIR)/%.spin
#	$(SPIN2DAT) -o $(basename $@) $<

%_firmware.o: %.dat
	$(OBJCOPY) -I binary -B propeller -O $(CC) $< $@

