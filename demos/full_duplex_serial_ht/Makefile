# #########################################################
# This makefile allows building and cleaning the 
# full duplex serial HT demo.
#
# Copyright (c) 2011 Michael Rychlik
# MIT Licensed
# #########################################################

# Propeller Demo include
PROPLIB = ../common

CFLAGS = -Os -mlmm

#
# objects for this program
#
# <HEATER>What's the thing with the "_1" naming convention ?</HEATER>
NAME = full_duplex_serial_ht_demo 
OBJS = full_duplex_serial_ht_demo.o full_duplex_serial_ht.o full_duplex_serial_ht_1.cog

all: $(NAME).elf

include $(PROPLIB)/common.mk

#
# do a partial link of all driver code
#

# # Note that calling our output file *_1.cog ensures that it
# is placed in the .coguser1 section by the main program's
# linker script; similarly *_2.cog would be placed in .coguser2,
# and so on

# the raw C code for the cog
full_duplex_serial_ht_1.o: full_duplex_serial_ht_cog.c
	$(CC) -r -Os -mcog -o $@ $<

#
# We have to avoid conflicts between symbols in the main C program and
# symbols in the local cog C programs. We do this by using objcopy to
# turn all the global symbols in the cog .text segment into local symbols
# (that's what --localize-text does).
#

%.cog: %.o
	$(OBJCOPY) --localize-text $^ $@


