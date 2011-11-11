#
# Makefile for Propeller C library
#

PREFIX = /usr/local/propeller
DEST = $(PREFIX)/propeller-elf

ifdef BUILD
OBJDIR=$(BUILD)/lib/obj
else
OBJDIR=../../build/lib/obj
endif

CC=propeller-elf-gcc
MKDIR = mkdir -p

VPATH=.:cog:misc:stdio:stdlib:string:sys:sys/propeller:time:drivers:fdlibm

all:	$(OBJDIR)
	make -f Makefile.incl MODEL=lmm OBJDIR=$(OBJDIR)
	make -f Makefile.incl MODEL=xmm OBJDIR=$(OBJDIR)
	make -f Makefile.incl MODEL=xmmc OBJDIR=$(OBJDIR)
	make -f Makefile.cog OBJDIR=$(OBJDIR)

hello.elf: hello.c
	$(CC) -o hello.elf hello.c

install: all
	mkdir -p $(DEST)/lib/xmm/
	mkdir -p $(DEST)/lib/xmmc/
	cp -r include $(DEST)
	cp $(OBJDIR)/lmm/libc.a $(OBJDIR)/lmm/libm.a $(DEST)/lib
	cp $(OBJDIR)/xmm/libc.a $(OBJDIR)/xmm/libm.a $(DEST)/lib/xmm/
	cp $(OBJDIR)/xmmc/libc.a $(OBJDIR)/xmmc/libm.a $(DEST)/lib/xmmc/
	cp $(OBJDIR)/cog/crt0_cog.o $(OBJDIR)/cog/crtend_cog.o $(DEST)/lib/
	cp $(OBJDIR)/cog/libcog.a $(DEST)/lib/

clean:
	rm -rf $(OBJDIR) *.a

$(OBJDIR):
	mkdir -p $(OBJDIR)

