# #########################################################
# This makefile allows building and cleaning the TvDemo
#
# To build with LMM model, enter $ make clean; make
# To build with XMM model, enter $ make clean; make MODEL=xmm
# To build with XMMC model, enter $ make clean; make MODEL=xmmc
#
# Copyright (c) 2011 Parallax, Inc.
# All rights MIT licensed
# #########################################################

# Common make
COMMON = ../../common

CC = propeller-elf-gcc
OBJCOPY = propeller-elf-objcopy

# flags to use for building
CFLAGS = -Os

#
# objects for this program
#

NAME = TvDemo
OBJS = TvDemo.o TvText.o TV_firmware.o TvDriver.o

all: $(NAME).elf

#
# include the fragment that defines most of everything
#
include $(COMMON)/common.mk
