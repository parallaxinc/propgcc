# #########################################################
# This makefile builds the fft_bench program
#   
# Copyright (c) 2011 Steve Denson
# All rights MIT licensed
# #########################################################

# Path to shared demo library
PROPLIB = ../common

#
# name and objects for this program
#

NAME = fft_bench
OBJS = fft_bench.o

# flags to build with
#CFLAGS = -Os
#CFLAGS = -Os -fno-inline-functions-called-once -mfcache
CFLAGS = -O2 -mfcache

# default build target
all: $(NAME).elf

#
# include the fragment that defines most of everything
#
include $(PROPLIB)/lmmdemo.mk
