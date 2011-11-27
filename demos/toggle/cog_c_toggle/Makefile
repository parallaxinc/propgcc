# #########################################################
# this makefile allows building and cleaning the toggle demo
#
# Copyright (c) 2011 Steve Denson
# MIT Licensed
# #########################################################

# Propeller Demo include
PROPLIB = ../../common

CFLAGS = -Os -mlmm
NM = propeller-elf-nm

#
# objects for this program
#

NAME = toggle
OBJS = toggle.o toggle_1.cog

all: $(NAME).elf

include $(PROPLIB)/lmmdemo.mk

#
# do a partial link of all driver code
#

# # Note that calling our output file *_1.cog ensures that it
# is placed in the .coguser1 section by the main program's
# linker script; similarly *_2.cog would be placed in .coguser2,
# and so on

toggle_1.o: toggle_cog.c
	$(CC) -r -Os -mcog -o $@ $<


#
# We have to avoid conflicts between symbols in the main C program and
# symbols in the local cog C programs. We do this by using objcopy to
# turn all the global symbols in the cog .text segment into local symbols
# (that's what --localize-text does).
#

%.cog: %.o
	$(OBJCOPY) --localize-text $^ $@

