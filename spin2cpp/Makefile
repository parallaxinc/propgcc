#
# Makefile for spin compiler
# Copyright (c) 2011 Total Spectrum Software Inc.
#

ifndef BUILDROOT
BUILDROOT=.
endif

ifndef TARGET
TARGET=/opt/parallax
endif

ifeq ($(OS),linux)
EXT=
endif

ifeq ($(OS),cygwin)
EXT=.exe
endif

ifeq ($(OS),msys)
EXT=.exe
endif

ifeq ($(OS),macosx)
EXT=
endif

OBJDIR=$(BUILDROOT)/obj/$(OS)
INSTALLDIR=$(TARGET)/bin

DIRS = $(OBJDIR) $(INSTALLDIR)

# these are to find the output of bison
vpath %.c $(OBJDIR)
vpath %.h $(OBJDIR)

#
# WARNING: byacc probably will not work!
#
#YACC = byacc
YACC = bison
CC = gcc
#CFLAGS = -g -Wall -Werror
#CC = clang
CFLAGS = -O -g -Wall -Werror -I$(OBJDIR) -I.
LIBS = -lm
RM = rm -f
MKDIR=mkdir -p
CP=cp

_SPIN2CPP = spin2cpp$(EXT)
_PROGS = testlex$(EXT) $(_SPIN2CPP)
_LEXOBJS = lexer.o symbol.o ast.o expr.o flexbuf.o preprocess.o
_OBJS = $(_LEXOBJS) spin.tab.o functions.o pasm.o outcpp.o outdat.o

SPIN2CPP = $(addprefix $(OBJDIR)/, $(_SPIN2CPP))
PROGS = $(addprefix $(OBJDIR)/, $(_PROGS))
LEXOBJS = $(addprefix $(OBJDIR)/, $(_LEXOBJS))
OBJS = $(addprefix $(OBJDIR)/, $(_OBJS))

all: $(OBJDIR) $(PROGS)

$(OBJDIR)/testlex$(EXT): testlex.c $(LEXOBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(OBJDIR)/spin.tab.c $(OBJDIR)/spin.tab.h: spin.y
	$(YACC) -t -b $(OBJDIR)/spin -d spin.y

lexer.c: $(OBJDIR)/spin.tab.h

clean: $(OBJDIR)
	$(RM) -r $(OBJDIR)

test: $(PROGS)
	$(OBJDIR)/testlex
	(cd Test; ./runtests.sh ../$(SPIN2CPP))

$(OBJDIR)/spin2cpp$(EXT): spin2cpp.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(OBJDIR)/%.o:	%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:	install
install:	all $(INSTALLDIR)
	$(CP) -f $(OBJDIR)/spin2cpp$(EXT) $(INSTALLDIR)

$(DIRS):
	$(MKDIR) $@
