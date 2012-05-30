CC=gcc
PROPCC=propeller-elf-gcc
ECHO=echo
RM=rm
LOAD=propeller-load

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

OBJS=gdbstub.o config.o expr.o system.o port.o PLoadLib.o $(foreach x, $(OSINT), $(x).o) kernel_image.o

all:	bin2c$(EXT) $(TARGET)

$(TARGET):	$(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

%.o:	%.c
	$(CC) -c $(CFLAGS) -o $@ $<

################
# DUMMY KERNEL #
################

kernel_image.elf:	kernel.c
	$(PROPCC) -Os -mcog -fomit-frame-pointer -o $@ $<

###########################
# C TO BINARY IMAGE ARRAY #
###########################

%_binary.elf:	%.c
	@$(ECHO) $@
	@$(PROPCC) -mlmm -g -o $@ $< 

%.binary:	%.elf
	@$(LOAD) -s $<
	@$(ECHO) $@

%.c:	%.binary
	./bin2c$(EXT) $< $@
	@$(ECHO) $@

#########
# TOOLS #
#########

bin2c$(EXT):	$(LOADERSRC)/tools/bin2c.c
	@$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@
	@$(ECHO) $@

.PHONY:	clean
clean:
	$(RM) -rf *.o $(TARGET) bin2c *.log kernel_image.*
