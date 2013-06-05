# #########################################################
# This makefile fragment builds LMM/XMM/XMMC demo programs
#   
# To use it, define:
#  NAME1/NAME2 to be the name of executables
#    - this is used to create the final program $(NAMEn).elf
#  LOBJS to be the object files needed for the library
#  OBJS1 to be the object files needed for the $(NAME1).elf
#  OBJS2 to be the object files needed for the $(NAME2).elf
#  MODEL to lmm, xmm, or xmmc
#  CFLAGS to be desired CFLAGS
#
#  Then set up a default "all" target - normally this will be:
#    all: $(NAME).elf
#  and finally:
#    include ./common.mk
#
# Based on the toggle makefiles:
# Copyright (c) 2011 Parallax Inc.
# All rights MIT licensed
# #########################################################

# where we installed the propeller binaries and libraries
PREFIX = /usr/local/propeller

ifndef MODEL
MODEL=lmm
endif

ifndef BOARD
BOARD=$(PROPELLER_LOAD_BOARD)
endif

ifneq ($(BOARD),)
BOARDFLAG=-b$(BOARD)
endif

CFLAGS += -m$(MODEL)
LDFLAGS += -m$(MODEL)

ifneq ($(LDSCRIPT),)
LDFLAGS += -T $(LDSCRIPT)
endif

CXXFILTFLAGS += --strip-underscore

# basic gnu tools
ifeq ($(CC),cc)
CC = propeller-elf-gcc
endif
ifeq ($(CXX),g++)
CXX = propeller-elf-g++
endif
ifeq ($(CXX),cc)
CXX = propeller-elf-gcc
endif
CXXFILT = propeller-elf-c++filt
LD = propeller-elf-ld
AS = propeller-elf-as
AR = propeller-elf-ar
OBJCOPY = propeller-elf-objcopy
LOADER = propeller-load
DUMP = propeller-elf-objdump
MAP_FIXER = propgcc-map-sizes.pl

# BSTC program
BSTC=bstc
SPINDIR=.

ifneq ($(NAME1),)
$(NAME1).elf $(NAME1).rawmap: $(OBJS1) $(BUILDLIB)
	$(CC) $(LDFLAGS)  -Xlinker -Map=$(NAME1).rawmap -o $@ $(OBJS1) $(LIBS) $(STRIP)

$(NAME1).dump: $(NAME1).elf
	$(DUMP) -d -S $(NAME1).elf | $(CXXFILT) $(CXXFILTFLAGS) > $(NAME1).dump

$(NAME1).map: $(NAME1).rawmap
	-$(CXXFILT) $(CXXFILTFLAGS) < $(NAME1).rawmap | perl $(MAP_FIXER) > $(NAME1).map
endif

ifneq ($(NAME2),)
$(NAME2).elf $(NAME2).rawmap: $(OBJS2) $(BUILDLIB)
	$(CC) $(LDFLAGS)  -Xlinker -Map=$(NAME2).rawmap -o $@ $(OBJS2) $(LIBS) $(STRIP)

$(NAME2).dump: $(NAME2).elf
	$(DUMP) -d -S $(NAME2).elf | $(CXXFILT) $(CXXFILTFLAGS) > $(NAME2).dump

$(NAME2).map: $(NAME2).rawmap
	-$(CXXFILT) $(CXXFILTFLAGS) < $(NAME2).rawmap | perl $(MAP_FIXER) > $(NAME2).map
endif

ifneq ($(LIBNAME),)
lib$(LIBNAME).a: $(LOBJS)
	$(AR) rs $@ $(LOBJS)
endif

%.o: %.c $(MAKES)
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.cpp $(MAKES)
	$(CC) $(CXXFLAGS) -o $@ -c $<

%.o: %.s $(MAKES)
	$(CC) -o $@ -c $<

#
# a .cog program is an object file that contains code intended to
# run in a COG separate from the main program; i.e., it's a COG
# driver
#
%.cog: %.c
	$(CC) $(CFLAGS) -r -mcog -o $@ $<
	$(OBJCOPY) --localize-text --rename-section .text=$@ $@
 
%.binary: %.elf
	$(LOADER) -s $<

%.dat: $(SPINDIR)/%.spin
	$(BSTC) -Ox -c -o $(basename $@) $<

%_firmware.o: %.dat
	$(OBJCOPY) -I binary -B propeller -O $(CC) $< $@

clean:
	rm -f *.o
	rm -f *.i
	rm -f *.ii
	rm -f *.s
	rm -f *.elf
	rm -f *.a
	rm -f *.cog
	rm -f *.map
	rm -f *.rawmap
	rm -f *.dump
	rm -f *.binary
	rm -f TAGS

#
# how to run
#
run: $(NAME).elf
	$(LOADER) $(BOARDFLAG) $(RUNELF) $(RUNFLAGS)
