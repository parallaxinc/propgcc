# #########################################################
# this makefile allows building and cleaning the toggle demo
#
# Copyright (c) 2011 Steve Denson
# MIT Licensed
# #########################################################

# Propeller Demo include
PROPLIB = ../../libpropeller

CFLAGS = -Os -mlmm

#
# objects for this program
#

NAME = toggle
OBJS = toggle.o toggle_1.cog $(PROPLIB)/libpropeller.a

all: $(NAME).elf

include $(PROPLIB)/lmmdemo.mk

#
# do a partial link of all driver code
#

# the raw C code for the cog
toggle_cog.o: toggle_cog.c
	$(CC) -Os -mcog -I$(PROPLIB)/../include -o $@ -c $<

# link the cog C code together with support libraries
# Note that calling our output file *_1.cog ensures that it
# is placed in the .coguser1 section by the main program's
# linker script; similarly *_2.cog would be placed in .coguser2,
# and so on

toggle_1.cog: toggle_cog.o
	$(LD) -s -o $@ -T $(PROPLIB)/propeller_cog.script $(PROPLIB)/crt0_cog.o $^ $(PROPLIB)/libcog.a


