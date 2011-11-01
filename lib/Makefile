#
# Makefile for Propeller C library
#
PREFIX = /usr/local/propeller
MKDIR = mkdir -p

ifdef BUILD
OBJDIR=$(BUILD)/lib/obj
else
OBJDIR=../../build/lib/obj
endif

VPATH=.:misc:stdio:stdlib:string:sys:sys/propeller:time:drivers:fdlibm

all:	$(OBJDIR)
	make -f Makefile.incl MODEL=lmm OBJDIR=$(OBJDIR)
	make -f Makefile.incl MODEL=xmm OBJDIR=$(OBJDIR)
	make -f Makefile.incl MODEL=xmmc OBJDIR=$(OBJDIR)

hello.elf: hello.c
	$(CC) -o hello.elf hello.c

install: all
	mkdir -p $(PREFIX)/propeller-elf/lib/xmm/
	mkdir -p $(PREFIX)/propeller-elf/lib/xmmc/
	cp -r include $(PREFIX)/propeller-elf
	cp $(OBJDIR)/lmm/libc.a $(OBJDIR)/lmm/libm.a $(PREFIX)/propeller-elf/lib
	cp $(OBJDIR)/xmm/libc.a $(OBJDIR)/xmm/libm.a $(PREFIX)/propeller-elf/lib/xmm/
	cp $(OBJDIR)/xmmc/libc.a $(OBJDIR)/xmmc/libm.a $(PREFIX)/propeller-elf/lib/xmmc/

clean:
	rm -rf $(OBJDIR) *.a

$(OBJDIR):
	mkdir -p $(OBJDIR)
