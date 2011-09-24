# #########################################################
# This makefile fragment builds LMM/XMM/XMMC demo programs
#   
# To use it, define:
#  PROPLIB to be the path to this directory
#  NAME to be the name of project
#       - this is used to create the final program $(NAME).elf
#  OBJS to be the object files needed for the project
#  MODEL to lmm, xmm, or xmmc
#  CFLAGS to be desired CFLAGS
#
#  Then set up a default "all" target (normally this will be
#    all: $(NAME).elf
#  and finally
#    include $(PROPLIB)/demo.mk
#
# Copyright (c) 2011 Parallax Inc.
# All rights MIT licensed
# #########################################################

# where we installed the propeller binaries and libraries
PREFIX = /usr/local/propeller

# libgcc directory
LIBGCC = $(PREFIX)/lib/gcc/propeller-elf/4.6.1

ifndef MODEL
MODEL=lmm
endif

ifndef PORT
PORT=1
endif

ifndef BOARD
BOARD=hub
endif

ifeq ($(MODEL),lmm)
STARTOBJ=spinboot.o
KERNELOBJ=crt0_lmm.o
BEGINOBJ=crtbegin_lmm.o
ENDOBJ=crtend_lmm.o
endif

ifeq ($(MODEL),xmm)
STARTOBJ=hubstart_xmm.o
KERNELOBJ=crt0_xmm.o
BEGINOBJ=crtbegin_xmm.o
ENDOBJ=crtend_xmm.o
endif

ifeq ($(MODEL),xmmc)
STARTOBJ=hubstart_xmm.o
KERNELOBJ=crt0_xmm.o
BEGINOBJ=crtbegin_xmm.o
ENDOBJ=crtend_xmm.o
endif

ifeq ($(STARTOBJ),)
$(error Unknown memory model '$(MODEL)'. Expecting lmm, xmm, or xmmc)
endif

STARTOBJS = -L$(LIBGCC) $(LIBGCC)/$(STARTOBJ) $(LIBGCC)/$(KERNELOBJ) $(LIBGCC)/$(BEGINOBJ)
ENDOBJS = -lgcc $(LIBGCC)/$(ENDOBJ)

# Current LIBRARY and INCL
PROPINC = $(PROPLIB)/../include

CFLAGS += -I$(PROPINC) -m$(MODEL)
LDSCRIPT = $(PROPLIB)/propeller_$(MODEL).script

# basic gnu tools
CC = propeller-elf-gcc
CXX = propeller-elf-g++
LD = propeller-elf-ld
AS = propeller-elf-as
OBJCOPY = propeller-elf-objcopy
CHKSUM = propeller-checksum
LOADER = propeller-load

CXXFLAGS += $(CFLAGS)

# BSTC program
BSTC=bstc -ls
SPINDIR=.
ECHO=echo

# C library to use
LIBC = $(PROPLIB)/libc.a

$(NAME).elf: $(LDSCRIPT) $(OBJS) $(LIBC)
	$(LD) -o $@ -T $(LDSCRIPT) $(STARTOBJS) $(OBJS) -lgcc $(LIBC) $(ENDOBJS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.s
	$(CC) -o $@ -c $<

%.binary: %.elf
	$(LOADER) -s $<

%.dat: $(SPINDIR)/%.spin
	$(BSTC) -Ox -c -o $(basename $@) $<
	$(ECHO) $@

%_firmware.o: %.dat
	$(OBJCOPY) -I binary -B propeller -O $(CC) $< $@
	rm -rf $<
	$(ECHO) $@

clean:
	rm -f *.o *.elf *.cog *.binary

#
# we make both libraries together
#
$(LIBC) $(PROPLIB)/libpropeller.a: .FORCE
	make -C $(PROPLIB)

#
# how to run
#
run: $(NAME).elf
	$(LOADER) -p$(PORT) -b$(BOARD) -I$(PROPLIB) $(NAME).elf -r -t

.FORCE:
