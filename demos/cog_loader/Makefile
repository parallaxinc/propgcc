# #########################################################
# this makefile allows building and cleaning the toggle demo
#
# Copyright (c) 2011 Steve Denson
# MIT Licensed
# #########################################################

# Propeller Demo include
PROPLIB = ../common

CFLAGS = -Os -DPARALLAX_I2C_BUS
LDFLAGS = -Wl,-Tcogloader.ld

#
# objects for this program
#

NAME = toggle

OBJS = \
toggle.o \
toggle_fw_0.eecog \
toggle_fw_1.eecog \
toggle_fw_2.eecog \
toggle_fw_3.eecog

all: $(NAME).elf

include $(PROPLIB)/common.mk
