# #########################################################
# This makefile builds the fft_bench program
#   
# Copyright (c) 2011 Steve Denson
# All rights MIT licensed
# #########################################################

# Current LIBRARY and INCL
PROPLIB = ../libpropeller

#
# objects for this program
#

NAME = fft_bench
OBJS = fft_bench.o clock.o stdio.o printf.o serial.o

CFLAGS = -Os

#
# include the fragment that defines most of everything
#

all: $(NAME).elf

include $(PROPLIB)/Makefile.incl

