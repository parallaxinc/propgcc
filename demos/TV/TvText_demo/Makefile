# #########################################################
# this makefile allows building and cleaning the TvDemo
#
# Copyright (c) 2011 Steve Denson
# All rights MIT licensed
# #########################################################

# where is the propeller library directory
PROPLIB = ../../libpropeller

# flags to use for building
CFLAGS = -Os

#
# objects for this program
#

NAME = TvDemo
OBJS = TvDemo.o TvText.o TV_firmware.o \
	clock.o stdio.o printf.o serial.o propeller.o

SPINDIR = .

all: $(NAME).elf

include $(PROPLIB)/Makefile.incl

%.dat: $(SPINDIR)/%.spin
	$(BSTC) -Ox -c -o $(basename $@) $<
	$(ECHO) $@

%_firmware.o: %.dat
	$(OBJCOPY) -I binary -B propeller -O $(CC) $< $@
	rm -rf $<
	$(ECHO) $@
