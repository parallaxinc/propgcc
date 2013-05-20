# #########################################################
# This makefile builds the Tiny library
#
# To build with LMM model, enter $ make clean; make
# To build with XMM model, enter $ make clean; make MODEL=xmm
# To build with XMMC model, enter $ make clean; make MODEL=xmmc
#   
# Based on the toggle makefiles:
# Copyright (c) 2011 Steve Denson
# All rights MIT licensed
# #########################################################

# Common make definitions
MAKES = Makefile

CC = propeller-elf-gcc
AR = propeller-elf-ar
MKDIR = mkdir -p

ifndef MODEL
MODEL=lmm
endif

ifndef MODELDIR
MODELDIR=obj
endif

#
# Name and objects for this program
#

LOBJS = serialparam.o \
        setbitcycles.o \
        putchar.o \
        getchar.o \
        fgetc.o \
        fputc.o \
        puts.o \
        doprintf.o \
        printf.o \
        sprintf.o \
        printf_putc.o \
        printf_puts.o \
        printf_putn.o \
        printf_putl.o \
        printf_pad.o \
        putstr.o \
        puthex.o \
        putdec.o \
        putudec.o \
        putfnum.o \
        printf_s.o \
        doprintf_s.o \
        printf_putls.o \
        printf_putll.o \
        putlhex.o \
        putldec.o \
        putludec.o \
        putlfnum.o \
        gets.o \
        safe_gets.o \
        scanf.o \
        sscanf.o \
        doscanf.o \
        scanf_getl.o \
        gethex.o \
        getdec.o \
        getudec.o \
        getfnum.o \
        scanf_getll.o \
        getlhex.o \
        getldec.o \
        getludec.o \
        getlfnum.o \

# these require c++ which isn't working yet on the propeller 2
#        c++-alloc.o \
#        tinystream.o

TINYDIR = $(MODELDIR)/tiny
TINY_LOBJS = $(addprefix $(TINYDIR)/, $(LOBJS))

# Flags to build with; customize these as necessary...

# This optimizes for both speed and small size
CFLAGS = -Wall -g -Os -mfcache -fno-exceptions -m$(MODEL)
CXXFLAGS = $(CFLAGS) -fno-rtti 

# dependencies
tinystream.o:  tinystream.h
$(TINY_LOBJS): serialparam.h siodev.h tinyio.h

# build libtiny.a
lib: $(TINYDIR) $(MODELDIR)/libtiny.a
$(MODELDIR)/libtiny.a : $(TINY_LOBJS)
	$(AR) rs $@ $(TINY_LOBJS)

$(TINYDIR)/%.o: %.c $(MAKES)
	$(CC) $(CFLAGS) -o $@ -c $<

$(TINYDIR)/%.o: %.cpp $(MAKES)
	$(CC) $(CXXFLAGS) -o $@ -c $<

TAGS: *.c *.h
	etags $^

$(TINYDIR):
	$(MKDIR) $@

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
