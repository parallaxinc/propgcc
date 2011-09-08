###################
# XBASIC Makefile #
###################

SRCDIR=src
SPINDIR=spin
OBJDIR=obj/$(OS)
BINDIR=bin/$(OS)
DRVDIR=include
INSTALLDIR=/usr/local/propeller
INSTALLBINDIR=$(INSTALLDIR)/bin
INSTALLLIBDIR=$(INSTALLDIR)/propeller-load

DIRS = $(OBJDIR) $(BINDIR) $(DRVDIR) $(INSTALLBINDIR) $(INSTALLLIBDIR)

CC=gcc
ECHO=echo
MKDIR=mkdir -p
CP=cp
RM=rm

CFLAGS=-Wall -I$(SRCDIR)/common -I$(SRCDIR)/runtime -I$(SRCDIR)/loader
LDFLAGS=$(CFLAGS)
SPINFLAGS=-Ogxr

ifeq ($(OS),linux)
CFLAGS += -DLINUX
BSTC=bstc.linux
EXT=
OSINT=osint_linux
endif

ifeq ($(OS),cygwin)
CFLAGS += -DCYGWIN
BSTC=bstc
EXT=.exe
OSINT=osint_cygwin
endif

ifeq ($(OS),macosx)
CFLAGS += -DMACOSX
BSTC=bstc.osx
EXT=
OSINT=osint_linux
endif

##################
# DEFAULT TARGET #
##################

.PHONY:	all
all:	propeller-load cache-drivers

#################
# CLEAN TARGETS #
#################

.PHONY:	clean clean-for-release
clean:
	@$(RM) -f -r $(OBJDIR)
	@$(RM) -f -r $(BINDIR)
	@$(RM) -f $(DRVDIR)/*.dat
	@$(RM) -f *.binary
	
.PHONY:
clean-all:	clean
	@$(RM) -f -r obj
	@$(RM) -f -r bin
	@$(RM) -f $(DRVDIR)/*.dat
	@$(RM) -f *.binary
	
#####################
# OBJECT FILE LISTS #
#####################

OBJS=\
$(OBJDIR)/propeller-load.o \
$(OBJDIR)/loader.o \
$(OBJDIR)/loadelf.o \
$(OBJDIR)/packet.o \
$(OBJDIR)/PLoadLib.o \
$(OBJDIR)/config.o \
$(OBJDIR)/system.o \
$(OBJDIR)/serial_helper.o \
$(OBJDIR)/flash_loader.o \
$(OBJDIR)/$(OSINT).o

HDRS=\
$(SRCDIR)/PLoadLib.h \
$(SRCDIR)/config.h \
$(SRCDIR)/loadelf.h \
$(SRCDIR)/loader.h \
$(SRCDIR)/osint.h \
$(SRCDIR)/packet.h \
$(SRCDIR)/system.h

############################################
# SOURCES NEEDED BY THE VISUAL C++ PROJECT #
############################################

HELPER_SRCS=\
$(OBJDIR)/serial_helper.c \
$(OBJDIR)/flash_loader.c

.PHONY:	spin-binaries
spin-binaries:	$(OBJDIR) bin2c $(HELPER_SRCS)

SPIN_SRCS=\
$(SPINDIR)/serial_helper.spin \
$(SPINDIR)/flash_loader.spin \
$(SPINDIR)/packet_driver.spin \
$(SPINDIR)/cache_interface.spin \
$(SPINDIR)/TV.spin \
$(SPINDIR)/TV_Text.spin \
$(SPINDIR)/c3_cache.spin \
$(SPINDIR)/ssf_cache.spin

#################
# CACHE DRIVERS #
#################

CACHE_DRIVERS=\
$(DRVDIR)/c3_cache.dat \
$(DRVDIR)/ssf_cache.dat

.PHONY:	cache-drivers
cache-drivers:	$(DRVDIR) $(CACHE_DRIVERS)

##################
# SPIN TO BINARY #
##################

$(OBJDIR)/serial_helper.binary:	$(SPINDIR)/serial_helper.spin $(SPIN_SRCS)
	@$(BSTC) $(SPINFLAGS) -b -o $(basename $@) $<
	@$(ECHO) $@

$(OBJDIR)/%_loader.binary:	$(SPINDIR)/%_loader.spin $(SPIN_SRCS)
	@$(BSTC) $(SPINFLAGS) -b -o $(basename $@) $<
	@$(ECHO) $@

$(OBJDIR)/%.c:	$(OBJDIR)/%.binary
	@$(BINDIR)/bin2c$(EXT) $< $@
	@$(ECHO) $@

###############
# SPIN TO DAT #
###############

$(DRVDIR)/%.dat:	$(SPINDIR)/%.spin $(SPIN_SRCS)
	@$(BSTC) $(SPINFLAGS) -c -o $(basename $@) $<
	@$(ECHO) $@

$(OBJDIR)/%.dat:	$(SPINDIR)/%.spin $(SPIN_SRCS)
	@$(BSTC) $(SPINFLAGS) -c -o $(basename $@) $<
	@$(ECHO) $@

$(OBJDIR)/%.c:	$(OBJDIR)/%.dat
	@$(BINDIR)/bin2c$(EXT) $< $@
	@$(ECHO) $@

################
# MAIN TARGETS #
################

.PHONY:	propeller-load
propeller-load:		$(BINDIR)/propeller-load$(EXT)

$(BINDIR)/propeller-load$(EXT):	$(BINDIR) $(OBJDIR) bin2c $(OBJS)
	@$(CC) $(LDFLAGS) $(OBJS) -o $@
	@$(ECHO) $@

#########
# RULES #
#########

$(OBJDIR)/%.o:	$(SRCDIR)/%.c $(HDRS)
	@$(CC) $(CFLAGS) -c $< -o $@
	@$(ECHO) $@

$(OBJDIR)/%.o:	$(OBJDIR)/%.c $(HDRS)
	@$(CC) $(CFLAGS) -c $< -o $@
	@$(ECHO) $@

#########
# TOOLS #
#########

.PHONY:	bin2c
bin2c:		$(BINDIR)/bin2c$(EXT)

$(BINDIR)/bin2c$(EXT):	$(OBJDIR) $(SRCDIR)/tools/bin2c.c
	@$(CC) $(CFLAGS) $(LDFLAGS) $(SRCDIR)/tools/bin2c.c -o $@
	@$(ECHO) $@

###############
# DIRECTORIES #
###############

$(DIRS):
	$(MKDIR) $@
	
##################
# INSTALL TARGET #
##################

.PHONY:	install
install:	$(INSTALLBINDIR) $(INSTALLLIBDIR)
	$(CP) -f bin/$(OS)/propeller-load $(INSTALLBINDIR)
	$(CP) -f include/* $(INSTALLLIBDIR)

##################
# RELEASE TARGET #
##################

.PHONY:	release
release:	clean-for-release
	$(RM) -rf ../loader-rel/loader
	$(MKDIR) ../loader-rel/loader
	$(CP) -r src ../loader-rel/loader
	$(CP) -r spin ../loader-rel/loader
	$(CP) -r include ../loader-rel/loader
	$(CP) makefile ../loader-rel/loader
	$(CP) setenv.* ../loader-rel/loader
	$(CP) README.txt ../loader-rel/loader
