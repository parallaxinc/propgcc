# #########################################################
# This makefile fragment builds LMM/XMM/XMMC demo programs
#   
# To use it, define:
#  PROPLIB to be the path to this directory
#  NAME to be the name of project
#       - this is used to create the final program $(NAME).elf
#  OBJS to be the object files needed for the project
#  MODEL to lmm, xmm, or xmmc
#  CFLAGS to be desired CFLAGS
#
#  Then set up a default "all" target (normally this will be
#    all: $(NAME).elf
#  and finally
#    include $(PROPLIB)/demo.mk
#
# Copyright (c) 2011 Parallax Inc.
# All rights MIT licensed
# #########################################################

# where we installed the propeller binaries and libraries
PREFIX = /opt/parallax

# libgcc directory
LIBGCC = $(PREFIX)/lib/gcc/propeller-elf/4.6.1

ifndef MODEL
MODEL=lmm
endif

ifndef BOARD
BOARD=$(PROPELLER_LOAD_BOARD)
endif

ifneq ($(BOARD),)
BOARDFLAG=-b$(BOARD)
endif

CFLAGS += -m$(MODEL)
LDFLAGS = -m$(MODEL)

# basic gnu tools
CC = propeller-elf-gcc
CXX = propeller-elf-g++
LD = propeller-elf-ld
AS = propeller-elf-as
OBJCOPY = propeller-elf-objcopy
LOADER = propeller-load

CXXFLAGS += $(CFLAGS)

# BSTC program
BSTC=bstc
SPINDIR=.


$(NAME).elf: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.s
	$(CC) -o $@ -c $<

%.o: %.sx
	$(CC) $(CFLAGS) -o $@ -c $<

%.binary: %.elf
	$(LOADER) -s $<

%.dat: $(SPINDIR)/%.spin
	$(BSTC) -Ox -c -o $(basename $@) $<

%_firmware.o: %.dat
	$(OBJCOPY) -I binary -B propeller -O $(CC) $< $@

clean:
	rm -f *.o *.elf *.cog *.binary

#
# how to run
#
run: $(NAME).elf
	$(LOADER) $(BOARDFLAG) $(NAME).elf -r -t

.FORCE:
