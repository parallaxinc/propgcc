###########################
# propeller-load Makefile #
###########################

ifndef BUILDROOT
BUILDROOT=.
endif

ifndef TARGET
TARGET=/opt/parallax
endif

SRCDIR=src
SPINDIR=spin
OBJDIR=$(BUILDROOT)/obj/$(OS)
BINDIR=$(BUILDROOT)/bin/$(OS)
DRVDIR=$(BUILDROOT)/include
INSTALLBINDIR=$(TARGET)/bin
INSTALLLIBDIR=$(TARGET)/propeller-load

DIRS = $(OBJDIR) $(PROPOBJDIR) $(BINDIR) $(DRVDIR) $(INSTALLBINDIR) $(INSTALLLIBDIR)

CC=gcc
ECHO=echo
MKDIR=mkdir -p
CP=cp
RM=rm

PROPCC=propeller-elf-gcc
PROPOBJCOPY=propeller-elf-objcopy
PROPCFLAGS=-mp2
PROPOBJDIR=$(BUILDROOT)/obj/propeller

ifeq ($(OS),linux)
CFLAGS += -DLINUX
BSTC_OS=bstc.linux
EXT=
OSINT=osint_linux
LIBS=
endif

ifeq ($(OS),cygwin)
CFLAGS += -DCYGWIN
BSTC_OS=bstc
EXT=.exe
OSINT=osint_cygwin enumcom
LIBS=-lsetupapi
endif

ifeq ($(OS),msys)
CFLAGS += -DMINGW
BSTC_OS=bstc
EXT=.exe
OSINT=osint_mingw enumcom
LIBS=-lsetupapi
endif

ifeq ($(OS),macosx)
CFLAGS += -DMACOSX
BSTC_OS=bstc
EXT=
OSINT=osint_linux
LIBS=
endif

CFLAGS+=-Wall -I$(SRCDIR)/common -I$(SRCDIR)/runtime -I$(SRCDIR)/loader
LDFLAGS=$(CFLAGS)

#
# Use the next two lines for bstc
#
SPIN=$(BSTC_OS) -Ogxr
SPINBFLAGS=
#SPIN=spin2cpp
#SPINBFLAGS=-mcmm -O

##################
# DEFAULT TARGET #
##################

.PHONY:	all

all:	info propeller-load drivers sd-loader

##########################
# SHOW BUILD INFORMATION #
##########################

info:
	@$(ECHO) CFLAGS: $(CFLAGS)
	@$(ECHO) LDFLAGS: $(LDFLAGS)
	@$(ECHO) SPIN: $(SPIN)

#################
# CLEAN TARGETS #
#################

.PHONY:	clean clean-for-release
clean:
	@$(RM) -f -r $(OBJDIR)
	@$(RM) -f -r $(BINDIR)
	@$(RM) -f $(DRVDIR)/*.dat $(DRVDIR)/*.elf
	@$(RM) -f *.binary

.PHONY:
clean-all:	clean
	@$(RM) -f -r $(BUILDROOT)/obj
	@$(RM) -f -r $(BUILDROOT)/bin
	@$(RM) -f $(DRVDIR)/*.dat
	@$(RM) -f *.binary

#####################
# OBJECT FILE LISTS #
#####################

OBJS=\
$(OBJDIR)/propeller-load.o \
$(OBJDIR)/loader.o \
$(OBJDIR)/lmm-image.o \
$(OBJDIR)/xmm-image.o \
$(OBJDIR)/pex-image.o \
$(OBJDIR)/loadelf.o \
$(OBJDIR)/packet.o \
$(OBJDIR)/PLoadLib.o \
$(OBJDIR)/p1image.o \
$(OBJDIR)/p2image.o \
$(OBJDIR)/p2loader.o \
$(OBJDIR)/config.o \
$(OBJDIR)/expr.o \
$(OBJDIR)/system.o \
$(OBJDIR)/port.o \
$(OBJDIR)/serial_helper.o \
$(OBJDIR)/flash_loader.o \
$(foreach x, $(OSINT), $(OBJDIR)/$(x).o)

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
$(SPINDIR)/ssf_cache.spin \
$(SPINDIR)/eeprom_cache.spin \
$(SPINDIR)/sd_cache.spin \
$(SPINDIR)/vm_start.spin

#################
# CACHE DRIVERS #
#################

DRIVERS=\
$(DRVDIR)/c3_cache.dat \
$(DRVDIR)/c3_cache_flash.dat \
$(DRVDIR)/ssf_cache.dat \
$(DRVDIR)/dracblade_cache.dat \
$(DRVDIR)/sdram_cache.dat \
$(DRVDIR)/eeprom_cache.dat \
$(DRVDIR)/sd_cache.dat \
$(DRVDIR)/spi_flash_cache.dat \
$(DRVDIR)/sst_spi_flash_cache.dat \
$(DRVDIR)/fast_spi_flash_cache.dat \
$(DRVDIR)/fast_sst_spi_flash_cache.dat \
$(DRVDIR)/spi_nway_flash_cache.dat \
$(DRVDIR)/sst_sqi_flash_cache.dat \
$(DRVDIR)/sst_sqi_nway_flash_cache.dat \
$(DRVDIR)/spi_sram_cache.dat \
$(DRVDIR)/spi_nway_sram_cache.dat \
$(DRVDIR)/winbond_sqi_flash_cache.dat \
$(DRVDIR)/winbond_sqi_nway_flash_cache.dat \
$(DRVDIR)/synapse_cache.dat \
$(DRVDIR)/rampage2_cache.dat

#################
# OTHER DRIVERS #
#################

DRIVERS+=\
$(DRVDIR)/sd_driver.dat

.PHONY:	drivers
drivers:	$(DRVDIR) $(DRIVERS)

.PHONY:	sd-loader
sd-loader:	$(DRVDIR)
	$(MAKE) -C sdloader BUILDROOT=$(realpath $(BUILDROOT))

##################
# SPIN TO BINARY #
##################

$(OBJDIR)/serial_helper.binary:	$(SPINDIR)/serial_helper.spin $(SPIN_SRCS)
	@$(SPIN) -b $(SPINBFLAGS) -o $(basename $@) $<
	@$(ECHO) $@

$(OBJDIR)/%_loader.binary:	$(SPINDIR)/%_loader.spin $(SPIN_SRCS)
	@$(SPIN) -b $(SPINBFLAGS) -o $(basename $@) $<
	@$(ECHO) $@

$(OBJDIR)/%.c:	$(OBJDIR)/%.binary
	@$(BINDIR)/bin2c$(EXT) $< $@
	@$(ECHO) $@

###############
# SPIN TO DAT #
###############

$(DRVDIR)/%.dat:	$(SPINDIR)/%.spin $(SPIN_SRCS)
	@$(SPIN) -c -DENABLE_RAM -o $(basename $@) $<
	@$(ECHO) $@

$(DRVDIR)/%_flash.dat:	$(SPINDIR)/%.spin $(SPIN_SRCS)
	@$(SPIN) -c -o $(basename $@) $<
	@$(ECHO) $@

$(DRVDIR)/sst_spi_flash_cache.dat:	$(SPINDIR)/spi_flash_cache.spin $(SPIN_SRCS)
	@$(SPIN) -c -DSST -o $(basename $@) $<
	@$(ECHO) $@

$(DRVDIR)/fast_spi_flash_cache.dat:	$(SPINDIR)/spi_flash_cache.spin $(SPIN_SRCS)
	@$(SPIN) -c -DFAST -o $(basename $@) $<
	@$(ECHO) $@

$(DRVDIR)/fast_sst_spi_flash_cache.dat:	$(SPINDIR)/spi_flash_cache.spin $(SPIN_SRCS)
	@$(SPIN) -c -DSST -DFAST -o $(basename $@) $<
	@$(ECHO) $@

$(DRVDIR)/sst_sqi_flash_cache.dat:	$(SPINDIR)/sqi_flash_cache.spin $(SPIN_SRCS)
	@$(SPIN) -c -DSST -o $(basename $@) $<
	@$(ECHO) $@

$(DRVDIR)/winbond_sqi_flash_cache.dat:	$(SPINDIR)/sqi_flash_cache.spin $(SPIN_SRCS)
	@$(SPIN) -c -DWINBOND -o $(basename $@) $<
	@$(ECHO) $@

$(DRVDIR)/sst_sqi_nway_flash_cache.dat:	$(SPINDIR)/sqi_nway_flash_cache.spin $(SPIN_SRCS)
	@$(SPIN) -c -DSST -o $(basename $@) $<
	@$(ECHO) $@

$(DRVDIR)/winbond_sqi_nway_flash_cache.dat:	$(SPINDIR)/sqi_nway_flash_cache.spin $(SPIN_SRCS)
	@$(SPIN) -c -DWINBOND -o $(basename $@) $<
	@$(ECHO) $@

$(OBJDIR)/%.dat:	$(SPINDIR)/%.spin $(SPIN_SRCS)
	@$(SPIN) -c -o $(basename $@) $<
	@$(ECHO) $@

############
# DAT TO C #
############

$(OBJDIR)/%.c:	$(OBJDIR)/%.dat
	@$(BINDIR)/bin2c$(EXT) $< $@
	@$(ECHO) $@

############
# GAS TO C #
############

$(PROPOBJDIR)/%.o:	$(SRCDIR)/%.s $(HDRS) $(PROPOBJDIR)
	@$(PROPCC) $(PROPCFLAGS) -nostdlib $< -o $@
	@$(ECHO) $(PROPCC) $@

$(PROPOBJDIR)/%.bin:	$(PROPOBJDIR)/%.o $(HDRS)
	@$(PROPOBJCOPY) -O binary $< $@
	@$(ECHO) cc $@

$(OBJDIR)/%.c:	$(PROPOBJDIR)/%.bin bin2c $(OBJDIR)
	@$(BINDIR)/bin2c$(EXT) $< $@
	@$(ECHO) bin2c $@

$(OBJDIR)/%.o:	$(OBJDIR)/%.c $(HDRS)
	@$(CC) $(CFLAGS) -c $< -o $@
	@$(ECHO) $(CC) $@

################
# MAIN TARGETS #
################

.PHONY:	propeller-load
propeller-load:		$(BINDIR)/propeller-load$(EXT)

$(BINDIR)/propeller-load$(EXT):	$(BINDIR) $(OBJDIR) bin2c $(OBJS)
	@$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
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
install:	all $(INSTALLBINDIR) $(INSTALLLIBDIR)
	$(CP) -f $(BUILDROOT)/bin/$(OS)/propeller-load $(INSTALLBINDIR)
	$(CP) -f $(DRVDIR)/* $(INSTALLLIBDIR)
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
