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

# where we installed the propeller binaries and libraries
PROPGCC = $(HOME)/Dropbox/parallax/propgcc
PROPINC = $(PROPGCC)/demos/include
PROPLIB = $(PROPGCC)/demos/libpropeller
LIBGCC = /usr/local/propeller/lib/gcc/propeller-elf/4.6.1

# libgcc directory
LIBGCCSRC = $(PROPGCC)/gcc/gcc/config/propeller

STARTOBJS = hubstart_xmm.o crt0_xmm.o crtbegin_xmm.o
ENDOBJS = crtend_xmm.o

CFLAGS += -I$(PROPINC)

LDSCRIPT = $(PROPLIB)/propeller_xmm_ram.script
LFLAGS = -L$(LIBGCC) -T$(LDSCRIPT)

# basic gnu tools
CC = propeller-elf-gcc
LD = propeller-elf-ld
AS = propeller-elf-as
OBJCOPY = propeller-elf-objcopy
LOADER = propeller-load

# BSTC program
BSTC=bstc -ls
SPINDIR=.
ECHO=echo

# C library to use
LIBC = $(PROPLIB)/libc.a

# default build target
all: $(NAME).elf

$(NAME).elf: $(LDSCRIPT) $(OBJS) $(STARTOBJS) $(ENDOBJS) $(LIBC)
	$(LD) -o $@ $(LFLAGS) $(STARTOBJS) $(OBJS) $(LIBC) $(ENDOBJS)

%.o: %.c
	$(CC) $(CFLAGS) -Os -mxmm -o $@ -c $<

%.o: $(LIBGCCSRC)/%.c
	$(CC) $(CFLAGS) -Os -mxmm -o $@ -c $<

%.o: %.s
	$(CC) -o $@ -c $<

%.o: $(LIBGCCSRC)/%.s
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
	rm -f *.o *.elf *.binary

#
# we make both libraries together
#
$(LIBC) $(PROPLIB)/libpropeller.a:
	make -C $(PROPLIB)

#
# how to run
#
run: #$(NAME).elf
	$(LOADER) -b c3 $(NAME).elf -r -t

