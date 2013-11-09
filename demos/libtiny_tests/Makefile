# #########################################################
# This makefile builds the Tiny library
#
# To build with LMM model, enter $ make clean; make
# To build with XMM model, enter $ make clean; make MODEL=xmm
# To build with XMMC model, enter $ make clean; make MODEL=xmmc
#   
# Based on the toggle makefiles:
# Copyright (c) 2011 Steve Denson
# All rights MIT licensed
# #########################################################

# Common make definitions
COMMON = .
COMMON_MAKE = $(COMMON)/common.mk
MAKES = Makefile $(COMMON_MAKE)

MODEL=lmm

#
# Name and objects for this program
#

NAME1 = ttinyio
OBJS1 = ttinyio.o

NAME2 = ttinyios
OBJS2 = ttinyios.o

OBJS = $(OBJS1) $(OBJS2)
LIBS = -ltiny

# Flags to build with; customize these as necessary...

# This optimizes for both speed and small size
CFLAGS = -Wall -g -Os -mfcache -fno-exceptions
CXXFLAGS = $(CFLAGS) -fno-rtti 

# This leaves temporary files around (so you can look at the generated code)
#CFLAGS += -save-temps

# Any extra linker flags
LDFLAGS = 

# Flags to run with (for propeller-load); customize as necessary)

# Change to your board
BOARD = quickstart

# Set to include your executable in the propeller-load command;
# Comment it out to use the -z and -l flags.
RUNELF = $(NAME).elf

# Use some combination of these:
#RUNFLAGS = -r -t      # Load ram and run, then open terminal
#RUNFLAGS = -r -t19200 # Load ram and run, then open terminal at 19200 baud
#RUNFLAGS = -e         # Burn to eeprom
#RUNFLAGS = -s         # Create .binary - don't load
#RUNFLAGS = -x         # Create .PEX file (for later use with -z and -l)
#RUNFLAGS = -z         # Run from SD card by using SD card as the cache (XMM only)

#RUNFLAGS = -l         # Run from SD card by loading from SD into cache (XMM and XMMC)
RUNFLAGS = -r -t       # Load ram and run, then open terminal

# default build target
all: test1 test2

# build the test programs
test1: $(NAME1).elf
test2: $(NAME2).elf

#
# include the fragment that defines most of everything
#
include $(COMMON_MAKE)
