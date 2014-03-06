
ifndef BUILDROOT
BUILDROOT=.
endif

CC=gcc
NATIVECC=gcc
PROPCC=propeller-elf-gcc
RM=rm
LOAD=propeller-load

LOADER=../loader
LOADERSRC=$(LOADER)/src

CFLAGS=-Wall -I$(LOADERSRC)
SPINFLAGS=-Ogxr

OBJDIR=$(BUILDROOT)

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

TARGET=$(BUILDROOT)/gdbstub$(EXT)

OBJS=$(OBJDIR)/gdbstub.o $(OBJDIR)/config.o $(OBJDIR)/expr.o \
	$(OBJDIR)/system.o \
	$(OBJDIR)/port.o \
	$(OBJDIR)/PLoadLib.o \
	$(foreach x, $(OSINT), $(OBJDIR)/$(x).o) \
	$(OBJDIR)/kernel_image.o

all:	dir bin2c$(EXT) $(TARGET)

dir:
	mkdir -p $(OBJDIR)

$(TARGET):	$(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

$(OBJDIR)/%.o:	%.c
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
	$(PROPCC) -mlmm -g -o $@ $< 

%.binary:	%.elf
	$(LOAD) -s $<

%.c:	%.binary
	./bin2c$(EXT) $< $@

#########
# TOOLS #
#########

bin2c$(EXT):	$(LOADERSRC)/tools/bin2c.c
	$(NATIVECC) $(CFLAGS) $(LDFLAGS) $< -o $@

.PHONY:	clean
clean:
	$(RM) -rf $(TARGET) $(OBJS) bin2c *.log kernel_image.*
	$(RM) -f obj
