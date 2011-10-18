# #########################################################
# This makefile builds the dhrystone program
#
# At the moment only the LMM model works.
#
# To build with LMM model, enter $ make clean; make
# To build with XMM model, enter $ make clean; make MODEL=xmm
# To build with XMMC model, enter $ make clean; make MODEL=xmmc
#   
# Copyright (c) 2011 Parallax, Inc.
# All rights MIT licensed
# #########################################################

# Common make
COMMON = ../common

#
# objects for this program
#

NAME = dry
OBJS = dry1.o dry2.o

# default build target
all: $(NAME).elf

# flags to use
# MSC_CLOCK selects the clock() function for timing
# printf=__simple_printf makes sure we use the small printf
# -DINTEGER_ONLY removes floating point

DEFINES = -Dprintf=__simple_printf -DMSC_CLOCK -DINTEGER_ONLY -DFIXED_NUMBER_OF_PASSES=5000
CFLAGS = -O2 -mfcache $(DEFINES)

#
# include the fragment that defines most of everything
#
include $(COMMON)/common.mk

dry1.o: dry.c
	$(CC) $(CFLAGS) -o $@ -c $<

dry2.o: dry.c
	$(CC) $(CFLAGS) -DPASS2 -o $@ -c $<

