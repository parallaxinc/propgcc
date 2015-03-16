#
# Makefile for Propeller C library
#

PREFIX ?= /opt/parallax

DEST = $(PREFIX)/propeller-elf

ifdef BUILD
OBJDIR=$(BUILD)/lib/obj
else
OBJDIR=../../build/lib/obj
endif

CC=propeller-elf-gcc
MKDIR = mkdir -p

VPATH=.:cog:misc:stdio:stdlib:string:sys:sys/propeller:time:drivers:wchar

all:	$(OBJDIR) cog .FORCE
	make -f Makefile.incl MODEL=lmm OBJDIR=$(OBJDIR)
	make -f Makefile.incl MODEL=lmm SHORTDOUBLES=1 OBJDIR=$(OBJDIR)
	make -f Makefile.incl MODEL=lmm CHIP=p2 OBJDIR=$(OBJDIR)
	make -f Makefile.incl MODEL=lmm CHIP=p2 SHORTDOUBLES=1 OBJDIR=$(OBJDIR)
	make -f Makefile.incl MODEL=cmm OBJDIR=$(OBJDIR)
	make -f Makefile.incl MODEL=cmm SHORTDOUBLES=1 OBJDIR=$(OBJDIR)
	make -f Makefile.incl MODEL=cmm CHIP=p2 OBJDIR=$(OBJDIR)
	make -f Makefile.incl MODEL=cmm CHIP=p2 SHORTDOUBLES=1 OBJDIR=$(OBJDIR)
	make -f Makefile.incl MODEL=xmmc OBJDIR=$(OBJDIR)
	make -f Makefile.incl MODEL=xmmc SHORTDOUBLES=1 OBJDIR=$(OBJDIR)
	make -f Makefile.incl MODEL=xmmc CHIP=p2 OBJDIR=$(OBJDIR)
	make -f Makefile.incl MODEL=xmmc CHIP=p2 SHORTDOUBLES=1 OBJDIR=$(OBJDIR)
	make -f Makefile.incl MODEL=xmm OBJDIR=$(OBJDIR)
	make -f Makefile.incl MODEL=xmm SHORTDOUBLES=1 OBJDIR=$(OBJDIR)
	make -f Makefile.incl MODEL=xmm CHIP=p2 OBJDIR=$(OBJDIR)
	make -f Makefile.incl MODEL=xmm CHIP=p2 SHORTDOUBLES=1 OBJDIR=$(OBJDIR)
	make -f Makefile.cog OBJDIR=$(OBJDIR)

cog:	$(OBJDIR) install-includes
	make -f Makefile.cog OBJDIR=$(OBJDIR)
	cp $(OBJDIR)/cog/crt0_cog.o $(OBJDIR)/cog/crtend_cog.o $(DEST)/lib/
	cp $(OBJDIR)/cog/libcog.a $(DEST)/lib/
	make -f Makefile.cog OBJDIR=$(OBJDIR)/p2 CHIP=-mp2 SUFFIX=2
	cp $(OBJDIR)/p2/cog/crt0_cog2.o $(OBJDIR)/p2/cog/crtend_cog2.o $(DEST)/lib/
	cp $(OBJDIR)/p2/cog/libcog2.a $(DEST)/lib/

# libtiny.a needs to be built after the c++ library
tiny:	$(OBJDIR)
	make -f Makefile.incl MODEL=lmm OBJDIR=$(OBJDIR) tiny
	make -f Makefile.incl MODEL=lmm SHORTDOUBLES=1 OBJDIR=$(OBJDIR) tiny
	make -f Makefile.incl MODEL=lmm CHIP=p2 OBJDIR=$(OBJDIR) tiny
	make -f Makefile.incl MODEL=lmm CHIP=p2 SHORTDOUBLES=1 OBJDIR=$(OBJDIR) tiny
	make -f Makefile.incl MODEL=cmm OBJDIR=$(OBJDIR) tiny
	make -f Makefile.incl MODEL=cmm SHORTDOUBLES=1 OBJDIR=$(OBJDIR) tiny
	make -f Makefile.incl MODEL=cmm CHIP=p2 OBJDIR=$(OBJDIR) tiny
	make -f Makefile.incl MODEL=cmm CHIP=p2 SHORTDOUBLES=1 OBJDIR=$(OBJDIR) tiny
	make -f Makefile.incl MODEL=xmmc OBJDIR=$(OBJDIR) tiny
	make -f Makefile.incl MODEL=xmmc SHORTDOUBLES=1 OBJDIR=$(OBJDIR) tiny
	make -f Makefile.incl MODEL=xmmc CHIP=p2 OBJDIR=$(OBJDIR) tiny
	make -f Makefile.incl MODEL=xmmc CHIP=p2 SHORTDOUBLES=1 OBJDIR=$(OBJDIR) tiny

	make -f Makefile.incl MODEL=xmm OBJDIR=$(OBJDIR) tiny
	make -f Makefile.incl MODEL=xmm SHORTDOUBLES=1 OBJDIR=$(OBJDIR) tiny
	make -f Makefile.incl MODEL=xmm CHIP=p2 OBJDIR=$(OBJDIR) tiny
	make -f Makefile.incl MODEL=xmm CHIP=p2 SHORTDOUBLES=1 OBJDIR=$(OBJDIR) tiny

install-tiny:
	cp $(OBJDIR)/lmm/libtiny.a $(DEST)/lib/
	cp $(OBJDIR)/lmm/short-doubles/libtiny.a $(DEST)/lib/short-doubles/
	cp $(OBJDIR)/lmm/p2/libtiny.a $(DEST)/lib/p2/
	cp $(OBJDIR)/lmm/p2/short-doubles/libtiny.a $(DEST)/lib/p2/short-doubles/
	cp $(OBJDIR)/cmm/libtiny.a $(DEST)/lib/cmm/
	cp $(OBJDIR)/cmm/short-doubles/libtiny.a $(DEST)/lib/cmm/short-doubles/
	cp $(OBJDIR)/cmm/p2/libtiny.a $(DEST)/lib/cmm/p2/
	cp $(OBJDIR)/cmm/p2/short-doubles/libtiny.a $(DEST)/lib/cmm/p2/short-doubles/
	cp $(OBJDIR)/xmmc/libtiny.a $(DEST)/lib/xmmc
	cp $(OBJDIR)/xmmc/short-doubles/libtiny.a $(DEST)/lib/xmmc/short-doubles
	cp $(OBJDIR)/xmmc/p2/libtiny.a $(DEST)/lib/xmmc/p2/
	cp $(OBJDIR)/xmmc/p2/short-doubles/libtiny.a $(DEST)/lib/xmmc/p2/short-doubles/
	cp $(OBJDIR)/xmm/libtiny.a $(DEST)/lib/xmm/
	cp $(OBJDIR)/xmm/short-doubles/libtiny.a $(DEST)/lib/xmm/short-doubles/
	cp $(OBJDIR)/xmm/p2/libtiny.a $(DEST)/lib/xmm/p2/
	cp $(OBJDIR)/xmm/p2/short-doubles/libtiny.a $(DEST)/lib/xmm/p2/short-doubles/
	cp tiny/tinyio.h tiny/tinystream.h tiny/tinystream tiny/siodev.h $(DEST)/include

hello.elf: hello.c
	$(CC) -o hello.elf hello.c

SRCLIBS = libc.a libm.a libpthread.a

install: all install-dirs install-includes install-specs
	cp $(addprefix $(OBJDIR)/lmm/, $(SRCLIBS)) $(DEST)/lib/
	cp $(addprefix $(OBJDIR)/lmm/short-doubles/, $(SRCLIBS)) $(DEST)/lib/short-doubles/
	cp $(addprefix $(OBJDIR)/lmm/p2/, $(SRCLIBS)) $(DEST)/lib/p2/
	cp $(addprefix $(OBJDIR)/lmm/p2/short-doubles/, $(SRCLIBS)) $(DEST)/lib/p2/short-doubles/
	cp $(addprefix $(OBJDIR)/cmm/, $(SRCLIBS)) $(DEST)/lib/cmm/
	cp $(addprefix $(OBJDIR)/cmm/short-doubles/, $(SRCLIBS)) $(DEST)/lib/cmm/short-doubles/
	cp $(addprefix $(OBJDIR)/cmm/p2/, $(SRCLIBS)) $(DEST)/lib/cmm/p2/
	cp $(addprefix $(OBJDIR)/cmm/p2/short-doubles/, $(SRCLIBS)) $(DEST)/lib/cmm/p2/short-doubles/
	cp $(addprefix $(OBJDIR)/xmmc/, $(SRCLIBS)) $(DEST)/lib/xmmc/
	cp $(addprefix $(OBJDIR)/xmmc/short-doubles/, $(SRCLIBS)) $(DEST)/lib/xmmc/short-doubles/
	cp $(addprefix $(OBJDIR)/xmmc/p2/, $(SRCLIBS)) $(DEST)/lib/xmmc/p2/
	cp $(addprefix $(OBJDIR)/xmmc/p2/short-doubles/, $(SRCLIBS)) $(DEST)/lib/xmmc/p2/short-doubles/
	cp $(addprefix $(OBJDIR)/xmm/, $(SRCLIBS)) $(DEST)/lib/xmm/
	cp $(addprefix $(OBJDIR)/xmm/short-doubles/, $(SRCLIBS)) $(DEST)/lib/xmm/short-doubles/
	cp $(addprefix $(OBJDIR)/xmm/p2/, $(SRCLIBS)) $(DEST)/lib/xmm/p2/
	cp $(addprefix $(OBJDIR)/xmm/p2/short-doubles/, $(SRCLIBS)) $(DEST)/lib/xmm/p2/short-doubles/
	cp LIB_LICENSE.txt $(DEST)/lib/
	cp ../gcc/COPYING3 ../gcc/COPYING.RUNTIME $(DEST)/lib/
	cp $(OBJDIR)/cog/crt0_cog.o $(OBJDIR)/cog/crtend_cog.o $(DEST)/lib/
	cp $(OBJDIR)/cog/libcog.a $(DEST)/lib/
	cp $(OBJDIR)/p2/cog/crt0_cog2.o $(OBJDIR)/p2/cog/crtend_cog2.o $(DEST)/lib/
	cp $(OBJDIR)/p2/cog/libcog2.a $(DEST)/lib/

install-dirs:
	mkdir -p $(DEST)/lib/short-doubles/
	mkdir -p $(DEST)/lib/p2/short-doubles/
	mkdir -p $(DEST)/lib/cmm/short-doubles/
	mkdir -p $(DEST)/lib/cmm/p2/short-doubles/
	mkdir -p $(DEST)/lib/xmm/short-doubles/
	mkdir -p $(DEST)/lib/xmm/p2/short-doubles/
	mkdir -p $(DEST)/lib/xmmc/short-doubles/
	mkdir -p $(DEST)/lib/xmmc/p2/short-doubles/

LIBSPECS = libgomp.spec

install-specs:
	cp $(LIBSPECS) $(DEST)/lib/
	cp $(LIBSPECS) $(DEST)/lib/short-doubles/
	cp $(LIBSPECS) $(DEST)/lib/cmm/
	cp $(LIBSPECS) $(DEST)/lib/cmm/short-doubles/
	cp $(LIBSPECS) $(DEST)/lib/p2/
	cp $(LIBSPECS) $(DEST)/lib/p2/short-doubles/
	cp $(LIBSPECS) $(DEST)/lib/xmm/
	cp $(LIBSPECS) $(DEST)/lib/xmm/short-doubles/
	cp $(LIBSPECS) $(DEST)/lib/xmm/p2/
	cp $(LIBSPECS) $(DEST)/lib/xmm/p2/short-doubles/
	cp $(LIBSPECS) $(DEST)/lib/xmmc/
	cp $(LIBSPECS) $(DEST)/lib/xmmc/short-doubles/
	cp $(LIBSPECS) $(DEST)/lib/xmmc/p2/
	cp $(LIBSPECS) $(DEST)/lib/xmmc/p2/short-doubles/

install-includes:
	cp -r include $(DEST)

clean:
	rm -rf $(OBJDIR) *.a

.FORCE:

$(OBJDIR):
	mkdir -p $(OBJDIR)

