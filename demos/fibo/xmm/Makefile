# #########################################################
# This makefile builds the fibo/xmm program
#   
# Copyright (c) 2011 Parallax, Inc.
# All rights MIT licensed
# #########################################################

# Path to shared demo library
PROPLIB = ../../libpropeller

#
# name and objects for this program
#

NAME = fibo
OBJS = fibo.o

# memory model
MODEL = lmm

# flags to build with
CFLAGS = -Os -mfcache

# default build target
all: $(NAME).elf

#
# include the fragment that defines most of everything
#
include $(PROPLIB)/demo.mk
