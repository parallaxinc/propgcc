MKDIR=mkdir -p
RM=rm
CC=cc
ECHO=echo

PROPCC=propeller-elf-gcc
PROPCFLAGS=-mp2
PROPOBJCOPY=propeller-elf-objcopy

OS?=macosx

ifeq ($(OS),linux)
CFLAGS+=-DLINUX
EXT=
OSINT=osint_linux
LIBS=
endif

ifeq ($(OS),cygwin)
CFLAGS+=-DCYGWIN
EXT=.exe
OSINT=osint_cygwin enumcom
LIBS=-lsetupapi
endif

ifeq ($(OS),msys)
CFLAGS += -DMINGW
EXT=.exe
OSINT=osint_mingw enumcom
LIBS=-lsetupapi
endif

ifeq ($(OS),macosx)
CFLAGS+=-DMACOSX
EXT=
OSINT=osint_linux
LIBS=
endif

ifeq ($(OS),)
$(error OS not set)
endif

SRCDIR=src
OBJDIR=obj/$(OS)
PROPOBJDIR=obj/propeller
BINDIR=bin/$(OS)

TARGET=$(BINDIR)/p2load$(EXT)

HDRS=

OBJS=\
$(OBJDIR)/p2load.o \
$(OBJDIR)/loadelf.o \
$(OBJDIR)/p2_loadimage.o \
$(OBJDIR)/loader.o \
$(foreach x, $(OSINT), $(OBJDIR)/$(x).o)

CFLAGS+=-Wall
LDFLAGS=$(CFLAGS)

.PHONY:	default
default:	$(TARGET)

DIRS=$(OBJDIR) $(PROPOBJDIR) $(BINDIR)

$(TARGET):	$(BINDIR) $(OBJDIR) $(OBJS)
	@$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
	@$(ECHO) link $@

$(OBJDIR)/%.o:	$(SRCDIR)/%.c $(HDRS) $(OBJDIR)
	@$(CC) $(CFLAGS) -c $< -o $@
	@$(ECHO) $(CC) $@

$(PROPOBJDIR)/%.o:	$(SRCDIR)/%.s $(HDRS) $(PROPOBJDIR)
	@$(PROPCC) $(PROPCFLAGS) -nostdlib $< -o $@
	@$(ECHO) $(PROPCC) $@

$(PROPOBJDIR)/%.bin:	$(PROPOBJDIR)/%.o $(HDRS)
	@$(PROPOBJCOPY) -O binary $< $@
	@$(ECHO) cc $@

$(OBJDIR)/%.c:	$(PROPOBJDIR)/%.bin bin2c $(OBJDIR)
	@$(BINDIR)/bin2c$(EXT) $< $@
	@$(ECHO) bin2c $@

.PHONY:	bin2c
bin2c:		$(BINDIR)/bin2c$(EXT)

$(BINDIR)/bin2c$(EXT):	$(SRCDIR)/tools/bin2c.c $(BINDIR)
	@$(CC) $(CFLAGS) $(LDFLAGS) $(SRCDIR)/tools/bin2c.c -o $@
	@$(ECHO) cc $@

$(DIRS):
	$(MKDIR) $@

.PHONY:	clean
clean:
	@$(RM) -f -r $(OBJDIR)
	@$(RM) -f -r $(PROPOBJDIR)
	@$(RM) -f -r $(BINDIR)

.PHONY:
clean-all:	clean
	@$(RM) -f -r obj
	@$(RM) -f -r bin
