# #########################################################
# This makefile fragment builds XMM demo programs
#   
# To use it, define:
#  PROPLIB to be the path to this directory
#  NAME to be the name of project
#       - this is used to create the final program $(NAME).elf
#  OBJS to be the object files needed for the project
#  CFLAGS to be desired CFLAGS
#
#  Then set up a default "all" target (normally this will be
#    all: $(NAME).elf
#  and finally
#    include $(PROPLIB)/lmmdemo.mk
#
# Copyright (c) 2011 Parallax Inc.
# All rights MIT licensed
# #########################################################

MODEL=xmmc
include $(PROPLIB)/demo.mk