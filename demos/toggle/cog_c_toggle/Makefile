# #########################################################
# this makefile allows building and cleaning the toggle demo
#
# Copyright (c) 2011 Steve Denson
# MIT Licensed
# #########################################################

# Propeller Demo include
PROPLIB = ../../libpropeller

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

# the raw C code for the cog
toggle_cog.o: toggle_cog.c
	$(CC) -Os -mcog -I$(PROPLIB)/../include -o $@ -c $<

# link the cog C code together with support libraries
# Note that calling our output file *_1.cog ensures that it
# is placed in the .coguser1 section by the main program's
# linker script; similarly *_2.cog would be placed in .coguser2,
# and so on

toggle_1.o: toggle_cog.o
	$(LD) -r -o $@ -T $(PROPLIB)/propeller_cog.script $(PROPLIB)/crt0_cog.o $^ $(PROPLIB)/libcog.a

#
# We have to avoid conflicts between symbols in the main C program and
# symbols in the local cog C programs. We do this by using objcopy to
# turn all the global symbols in the cog .text segment into local symbols
# (that's what --localize-symbols does). The only tricky bit is that
# --localize-symbols needs a list of symbols to localize given in a file;
# we construct that by using nm to find all symbols, grep to restrict it
# to global text symbols, and cut to strip off the addresses
#

%.cog: %.o
	$(NM) $^ | grep " T " | cut --bytes="12-" > local.lst
	$(OBJCOPY) --localize-symbols=local.lst $^ $@
	rm -f local.lst

