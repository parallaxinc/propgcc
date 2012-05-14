# #########################################################
# This makefile builds the filetest/xmm program
#   
# Copyright (c) 2011 Parallax, Inc.
# All rights MIT licensed
# #########################################################

# Path to shared demo library
PROPLIB = ../common

#
# name and objects for this program
#

NAME = filetest
OBJS = filetest.o

# memory model
MODEL ?= xmmc

# flags to build with
CFLAGS = -Os -mfcache

# where to find the source files
vpath %.c ..

# default build target
all: $(NAME).elf

#
# include the fragment that defines most of everything
#
include $(PROPLIB)/common.mk
