# #########################################################
# this makefile allows building and cleaning the toggle demo
#
# Copyright (c) 2011 Steve Denson
# MIT Licensed
# #########################################################

# Propeller Demo include
PROPLIB = ../common

CFLAGS = -save-temps -Os -DPARALLAX_I2C_BUS
CHIP = -mp2

#
# objects for this program
#

NAME = toggle

OBJS = \
toggle.o \
toggle_fw.cog

all: $(NAME).elf

include $(PROPLIB)/common.mk

run2:
	p2load toggle.elf
