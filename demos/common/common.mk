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

ifndef BOARD
BOARD=$(PROPELLER_LOAD_BOARD)
endif

ifneq ($(BOARD),)
BOARD=-b$(BOARD)
endif

CFLAGS += -m$(MODEL)
LDFLAGS += -m$(MODEL) -fno-exceptions -fno-rtti

ifneq ($(LDSCRIPT),)
LDFLAGS += -T $(LDSCRIPT)
endif

# basic gnu tools
CC = propeller-elf-gcc
CXX = propeller-elf-g++
LD = propeller-elf-ld
AS = propeller-elf-as
AR = propeller-elf-ar
OBJCOPY = propeller-elf-objcopy
LOADER = propeller-load

CXXFLAGS += $(CFLAGS)

# BSTC program
BSTC=bstc
SPINDIR=.
ECHO=echo

ifneq ($(NAME),)
$(NAME).elf: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
endif

ifneq ($(LIBNAME),)
lib$(LIBNAME).a: $(OBJS)
	$(AR) rs $@ $(OBJS)
endif

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.cpp
	$(CC) $(CXXFLAGS) -o $@ -c $<

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
	rm -f *.o *.elf *.a *.cog *.binary

#
# how to run
#
run: $(NAME).elf
	$(LOADER) $(BOARD) $(NAME).elf -r -t
