#
# Makefile for Propeller C library
#

ifndef PREFIX
PREFIX = /opt/parallax
endif

DEST = $(PREFIX)/propeller-elf

ifdef BUILD
OBJDIR=$(BUILD)/lib/obj
else
OBJDIR=../../build/lib/obj
endif

CC=propeller-elf-gcc
MKDIR = mkdir -p

VPATH=.:cog:misc:stdio:stdlib:string:sys:sys/propeller:time:drivers:wchar

all:	$(OBJDIR)
	make -f Makefile.incl MODEL=lmm OBJDIR=$(OBJDIR)
	make -f Makefile.incl MODEL=lmm SHORTDOUBLES=1 OBJDIR=$(OBJDIR)
	make -f Makefile.incl MODEL=xmmc OBJDIR=$(OBJDIR)
	make -f Makefile.incl MODEL=xmmc SHORTDOUBLES=1 OBJDIR=$(OBJDIR)
	make -f Makefile.incl MODEL=xmm OBJDIR=$(OBJDIR)
	make -f Makefile.incl MODEL=xmm SHORTDOUBLES=1 OBJDIR=$(OBJDIR)
	make -f Makefile.cog OBJDIR=$(OBJDIR)

hello.elf: hello.c
	$(CC) -o hello.elf hello.c

SRCLIBS = libc.a libm.a libpthread.a

install: all
	mkdir -p $(DEST)/lib/short-doubles/
	mkdir -p $(DEST)/lib/xmm/short-doubles/
	mkdir -p $(DEST)/lib/xmmc/short-doubles/
	cp -r include $(DEST)
	cp $(addprefix $(OBJDIR)/lmm/, $(SRCLIBS)) $(DEST)/lib/
	cp $(addprefix $(OBJDIR)/lmm/short-doubles/, $(SRCLIBS)) $(DEST)/lib/short-doubles/
	cp $(addprefix $(OBJDIR)/xmmc/, $(SRCLIBS)) $(DEST)/lib/xmmc/
	cp $(addprefix $(OBJDIR)/xmmc/short-doubles/, $(SRCLIBS)) $(DEST)/lib/xmmc/short-doubles/
	cp $(addprefix $(OBJDIR)/xmm/, $(SRCLIBS)) $(DEST)/lib/xmm/
	cp $(addprefix $(OBJDIR)/xmm/short-doubles/, $(SRCLIBS)) $(DEST)/lib/xmm/short-doubles/
	cp $(OBJDIR)/cog/crt0_cog.o $(OBJDIR)/cog/crtend_cog.o $(DEST)/lib/
	cp $(OBJDIR)/cog/libcog.a $(DEST)/lib/
	cp LIB_LICENSE.txt $(DEST)/lib/
	cp ../gcc/COPYING3 ../gcc/COPYING.RUNTIME $(DEST)/lib/

clean:
	rm -rf $(OBJDIR) *.a

$(OBJDIR):
	mkdir -p $(OBJDIR)

