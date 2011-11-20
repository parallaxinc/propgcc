# #########################################################
# This makefile builds the fft_bench program
#
# To build with LMM model, enter $ make clean; make
# To build with XMM model, enter $ make clean; make MODEL=xmm
# To build with XMMC model, enter $ make clean; make MODEL=xmmc
#   
# Copyright (c) 2011 Steve Denson
# All rights MIT licensed
# #########################################################

# Common make
COMMON = ../common

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
include $(COMMON)/common.mk
