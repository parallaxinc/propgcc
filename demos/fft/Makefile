# #########################################################
# This makefile builds the fft_bench program
#   
# Copyright (c) 2011 Steve Denson
# All rights MIT licensed
# #########################################################

# Path to shared demo library
PROPLIB = ../libpropeller

#
# name and objects for this program
#

NAME = fft_bench
OBJS = fft_bench.o

# flags to build with
CFLAGS = -Os -mexperimental

# default build target
all: $(NAME).elf

#
# include the fragment that defines most of everything
#
include $(PROPLIB)/lmmdemo.mk
