# #########################################################
# this makefile allows building and cleaning the toggle demo
#
# Copyright (c) 2011 Steve Denson
# MIT Licensed
# #########################################################

# Propeller Demo include
PROPLIB = ../common

CFLAGS = -Os -DPARALLAX_I2C_BUS

#
# objects for this program
#

NAME = toggle

OBJS = \
toggle.o \
toggle_fw_0.ecog \
toggle_fw_1.ecog \
toggle_fw_2.ecog \
toggle_fw_3.ecog

all: $(NAME).elf

include $(PROPLIB)/common.mk
