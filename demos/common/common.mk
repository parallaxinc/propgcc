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
PREFIX = /usr/local/propeller

ifndef MODEL
MODEL=lmm
endif

ifndef PORT
PORT=$(PROPELLER_LOAD_PORT)
endif

ifndef BOARD
BOARD=$(PROPELLER_LOAD_BOARD)
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
ECHO=echo

$(NAME).elf: $(OBJS)
	$(CC) $(LDFLAGS) $(LDSCRIPT) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.s
	$(CC) -o $@ -c $<

%.binary: %.elf
	$(LOADER) -s $<

%.dat: $(SPINDIR)/%.spin
	$(BSTC) -Ox -c -o $(basename $@) $<
	$(ECHO) $@

%_firmware.o: %.dat
	$(OBJCOPY) -I binary -B propeller -O $(CC) $< $@
	rm -rf $<
	$(ECHO) $@

clean:
	rm -f *.o *.elf *.cog *.binary

#
# how to run
#
run: $(NAME).elf
	$(LOADER) -p$(PORT) -b$(BOARD) $(NAME).elf -r -t
