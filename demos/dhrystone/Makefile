# #########################################################
# This makefile builds the fft_bench program
#   
# Copyright (c) 2011 Steve Denson
# All rights MIT licensed
# #########################################################

# Current LIBRARY
PROPLIB = ../libpropeller

#
# objects for this program
#

NAME = dry
OBJS = dry1.o dry2.o

# flags to use
# MSC_CLOCK selects the clock() function for timing
# printf=f_printf makes sure we use the floating point printf
# -DINTEGER_ONLY removes floating point

DEFINES = -DMSC_CLOCK -DINTEGER_ONLY -DFIXED_NUMBER_OF_PASSES=50000
CFLAGS = -O2 -mfcache $(DEFINES)

include $(PROPLIB)/lmmdemo.mk

dry1.o: dry.c
	$(CC) $(CFLAGS) -o $@ -c $<

dry2.o: dry.c
	$(CC) $(CFLAGS) -DPASS2 -o $@ -c $<

