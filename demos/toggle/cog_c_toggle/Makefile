# #########################################################
# this makefile allows building and cleaning the toggle demo
#
# Copyright (c) 2011 Steve Denson
# MIT Licensed
# #########################################################

# Propeller Demo include
PROPLIB = ../../common

#CFLAGS = -Os -mlmm
CFLAGS = -Os
NM = propeller-elf-nm

#
# objects for this program
#

NAME = toggle
OBJS = toggle.o toggle_fw.cog

all: $(NAME).elf

include $(PROPLIB)/common.mk

#
# do a partial link of all driver code
#
# Note that any sections called .cog* (or *.cog) are automatically
# set up as overlays in cog memory space by the linker.
# So to load a COG C program from an LMM program, we

toggle_fw.o: toggle_fw.c
	$(CC) -r -Os -mcog -o $@ $<


#
# We have to avoid conflicts between symbols in the main C program and
# symbols in the local cog C programs. We do this by using objcopy to
# turn all the global symbols in the cog .text segment into local symbols
# (that's what --localize-text does). At the same time we rename
# the .text section to something ending in .cog (the filename works!) so
# that the linker knows to treat it as an overlay.
#

%.cog: %.o
	$(OBJCOPY) --rename-section .text=$@ --localize-text $^ $@

