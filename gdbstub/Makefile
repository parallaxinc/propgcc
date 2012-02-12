CC=gcc
ECHO=echo
RM=rm

LOADER=../loader
LOADERSRC=$(LOADER)/src

CFLAGS=-Wall -I$(LOADERSRC)
SPINFLAGS=-Ogxr

vpath %.c $(LOADERSRC)
vpath %.h $(LOADERSRC)

ifeq ($(OS),linux)
CFLAGS += -DLINUX
BSTC=bstc.linux
EXT=
OSINT=osint_linux
LIBS=
endif

ifeq ($(OS),cygwin)
CFLAGS += -DCYGWIN
BSTC=bstc
EXT=.exe
OSINT=osint_cygwin enumcom
LIBS=-lsetupapi
endif

ifeq ($(OS),msys)
CFLAGS += -DMINGW
BSTC=bstc
EXT=.exe
OSINT=osint_mingw enumcom
LIBS=-lsetupapi
endif

ifeq ($(OS),macosx)
CFLAGS += -DMACOSX
BSTC=bstc.osx
EXT=
OSINT=osint_linux
LIBS=
endif

TARGET=gdbstub$(EXT)

OBJS=gdbstub.o config.o system.o port.o PLoadLib.o $(OSINT).o

all:    bin2c$(EXT) $(TARGET)

$(TARGET):	$(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o:	%.c
	$(CC) -c $(CFLAGS) -o $@ $<

#############
# SPIN TO C #
#############

%.binary:	%.spin
	@$(BSTC) $(SPINFLAGS) -b -o $(basename $@) $<
	@$(ECHO) $@

%.c:	%.binary
	@$(BINDIR)/bin2c$(EXT) $< $@
	@$(ECHO) $@

#########
# TOOLS #
#########

bin2c$(EXT):	$(LOADERSRC)/tools/bin2c.c
	@$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@
	@$(ECHO) $@

.PHONY:	clean
clean:
	$(RM) -rf *.o $(TARGET) bin2c *.log
