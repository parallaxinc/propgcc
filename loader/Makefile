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
CACHEDRIVERDIR=cache-drivers
OLDCACHEDRIVERDIR=old-cache-drivers
XMEMDRIVERDIR=xmem-drivers
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
SPINCMP=openspin.linux
CFLAGS += -DLINUX
EXT=
OSINT=osint_linux
LIBS=
endif

ifeq ($(OS),cygwin)
SPINCMP=openspin.exe
CFLAGS += -DCYGWIN
EXT=.exe
OSINT=osint_cygwin enumcom
LIBS=-lsetupapi
endif

ifeq ($(OS),msys)
SPINCMP=openspin.exe
CFLAGS += -DMINGW
EXT=.exe
OSINT=osint_mingw enumcom
LIBS=-lsetupapi
endif

ifeq ($(OS),macosx)
SPINCMP=openspin.osx
CFLAGS += -DMACOSX
EXT=
OSINT=osint_linux
LIBS=
endif

CFLAGS+=$(DEBUG) -Wall -I$(SRCDIR)/common -I$(SRCDIR)/runtime -I$(SRCDIR)/loader
LDFLAGS=$(CFLAGS)

# for compiling Spin code
SPINFLAGS=

# for compiling PASM external memory drivers
SPIN_DAT=spin2cpp --dat

##################
# DEFAULT TARGET #
##################

.PHONY:	all

all:	info propeller-load drivers sd-loader propeller-elf-image-size \
$(OBJDIR)/serial_helper.binary \
$(OBJDIR)/serial_helper2.binary


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
$(OBJDIR)/xmm-image2.o \
$(OBJDIR)/pex-image.o \
$(OBJDIR)/loadelf.o \
$(OBJDIR)/packet.o \
$(OBJDIR)/PLoadLib.o \
$(OBJDIR)/p1image.o \
$(OBJDIR)/p2image.o \
$(OBJDIR)/p2loader.o \
$(OBJDIR)/p2booter.o \
$(OBJDIR)/p2flasher.o \
$(OBJDIR)/config.o \
$(OBJDIR)/expr.o \
$(OBJDIR)/system.o \
$(OBJDIR)/port.o \
$(OBJDIR)/serial_helper.o \
$(OBJDIR)/serial_helper2.o \
$(OBJDIR)/flash_loader.o \
$(OBJDIR)/flash_loader2.o \
$(foreach x, $(OSINT), $(OBJDIR)/$(x).o)

IMAGE_SIZE_OBJS=\
$(OBJDIR)/propeller-elf-image-size.o \
$(OBJDIR)/loadelf.o

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
$(OBJDIR)/serial_helper2.c \
$(OBJDIR)/flash_loader.c

.PHONY:	spin-binaries
spin-binaries:	$(OBJDIR) bin2c $(HELPER_SRCS)

SPIN_SRCS=\
$(SPINDIR)/serial_helper.spin \
$(SPINDIR)/serial_helper2.spin \
$(SPINDIR)/flash_loader.spin \
$(SPINDIR)/flash_loader2.spin \
$(SPINDIR)/packet_driver.spin \
$(SPINDIR)/TV.spin \
$(SPINDIR)/TV_Text.spin \
$(SPINDIR)/vm_start.spin

OLD_CACHE_DRIVER_SRCS=\
$(OLDCACHEDRIVERDIR)/cache_interface.spin

#################
# CACHE DRIVERS #
#################

DRIVERS=\
$(DRVDIR)/c3_cache.dat \
$(DRVDIR)/c3_cache2.dat \
$(DRVDIR)/c3_cache_flash.dat \
$(DRVDIR)/ssf_cache.dat \
$(DRVDIR)/dracblade_cache.dat \
$(DRVDIR)/sdram_cache.dat \
$(DRVDIR)/eeprom_cache.dat \
$(DRVDIR)/sd_cache.dat \
$(DRVDIR)/spi_flash_cache.dat \
$(DRVDIR)/spi_flash_cache2.dat \
$(DRVDIR)/sst_spi_flash_cache.dat \
$(DRVDIR)/spi_nway_flash_cache.dat \
$(DRVDIR)/sst_sqi_flash_cache.dat \
$(DRVDIR)/sst_sqi_nway_flash_cache.dat \
$(DRVDIR)/spi_sram_cache.dat \
$(DRVDIR)/spi_sram24_cache.dat \
$(DRVDIR)/spi_nway_sram_cache.dat \
$(DRVDIR)/winbond_sqi_flash_cache.dat \
$(DRVDIR)/winbond_sqi_nway_flash_cache.dat \
$(DRVDIR)/synapse_cache.dat \
$(DRVDIR)/rampage2_cache.dat \
$(DRVDIR)/winbond_spi_flash_xmem.dat \
$(DRVDIR)/sst_spi_flash_xmem.dat \
$(DRVDIR)/spi_sram_xmem.dat \
$(DRVDIR)/spi_sram24_xmem.dat \
$(DRVDIR)/winbond_sqi_flash_xmem.dat \
$(DRVDIR)/sst_sqi_flash_xmem.dat \
$(DRVDIR)/sqi_sram_xmem.dat \
$(DRVDIR)/c3_xmem.dat \
$(DRVDIR)/eeprom_xmem.dat \
$(DRVDIR)/winbond_sqi_flash_sram_xmem.dat \
$(DRVDIR)/sst_sqi_flash_sram_xmem.dat \
$(DRVDIR)/rampage2_xmem.dat \
$(DRVDIR)/synapse_xmem.dat \
$(DRVDIR)/sd_xmem.dat

#######################
# NEWER CACHE DRIVERS #
#######################

DRIVERS+=\
$(DRVDIR)/rampage2_xcache.dat \
$(DRVDIR)/winbond_spi_flash_xcache.dat \
$(DRVDIR)/sst_spi_flash_xcache.dat \
$(DRVDIR)/winbond_sqi_flash_xcache.dat \
$(DRVDIR)/sst_sqi_flash_xcache.dat \
$(DRVDIR)/spi_sram_xcache.dat \
$(DRVDIR)/spi_sram24_xcache.dat \
$(DRVDIR)/sqi_sram_xcache.dat \
$(DRVDIR)/winbond_sqi_flash_sram_xcache.dat \
$(DRVDIR)/sst_sqi_flash_sram_xcache.dat \
$(DRVDIR)/c3_xcache.dat \
$(DRVDIR)/synapse_xcache.dat

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

$(OBJDIR)/%.binary:	$(SPINDIR)/%.spin $(SPIN_SRCS)
	@$(SPINCMP) -b $(SPINFLAGS) -o $@ $<
	@$(ECHO) $@

$(OBJDIR)/%.c:	$(OBJDIR)/%.binary
	@$(BINDIR)/bin2c$(EXT) $< $@
	@$(ECHO) $@

###############
# SPIN TO DAT #
###############

$(OBJDIR)/%.dat:	$(SPINDIR)/%.spin $(SPIN_SRCS)
	@$(SPIN_DAT) -o $@ $<
	@$(ECHO) $@

$(DRVDIR)/%.dat:	$(SPINDIR)/%.spin $(SPIN_SRCS)
	@$(SPIN_DAT) -o $@ $<
	@$(ECHO) $@

####################################
# RULES TO BUILD OLD CACHE DRIVERS #
####################################

$(DRVDIR)/%.dat:	$(OLDCACHEDRIVERDIR)/%.spin $(OLD_CACHE_DRIVER_SRCS)
	@$(SPIN_DAT) -DENABLE_RAM -o $@ $<
	@$(ECHO) $@

$(DRVDIR)/%_flash.dat:	$(OLDCACHEDRIVERDIR)/%.spin $(OLD_CACHE_DRIVER_SRCS)
	@$(SPIN_DAT) -o $@ $<
	@$(ECHO) $@

$(DRVDIR)/sst_%.dat:	$(OLDCACHEDRIVERDIR)/%.spin $(OLD_CACHE_DRIVER_SRCS)
	@$(SPIN_DAT) -DSST -o $@ $<
	@$(ECHO) $@

$(DRVDIR)/winbond_%.dat:	$(OLDCACHEDRIVERDIR)/%.spin $(OLD_CACHE_DRIVER_SRCS)
	@$(SPIN_DAT) -DWINBOND -o $@ $<
	@$(ECHO) $@

######################################
# RULES TO BUILD NEWER CACHE DRIVERS #
######################################

CACHE_DRIVER_COMMON=\
$(CACHEDRIVERDIR)/cache_common.spin \
$(CACHEDRIVERDIR)/cache_interface.spin \
$(CACHEDRIVERDIR)/cache_spi_pins.spin \
$(CACHEDRIVERDIR)/cache_sqi_pins.spin \
$(CACHEDRIVERDIR)/cache_spi.spin \
$(CACHEDRIVERDIR)/cache_sqi.spin

$(DRVDIR)/%.dat:	$(CACHEDRIVERDIR)/%.spin $(CACHE_DRIVER_COMMON)
	@$(SPIN_DAT) -o $@ $<
	@$(ECHO) $@

$(DRVDIR)/winbond_%.dat:	$(CACHEDRIVERDIR)/%.spin $(CACHE_DRIVER_COMMON)
	@$(SPIN_DAT) -DWINBOND -o $@ $<
	@$(ECHO) $@

$(DRVDIR)/sst_%.dat:	$(CACHEDRIVERDIR)/%.spin $(CACHE_DRIVER_COMMON)
	@$(SPIN_DAT) -DSST -o $@ $<
	@$(ECHO) $@

##########################################
# RULES TO BUILD EXTERNAL MEMORY DRIVERS #
##########################################

XMEM_DRIVER_COMMON=\
$(XMEMDRIVERDIR)/xmem_common.spin

$(DRVDIR)/%_xmem.dat:	$(XMEMDRIVERDIR)/%_xmem.spin $(XMEM_DRIVER_COMMON)
	@$(SPIN_DAT) -o $@ $<
	@$(ECHO) $@

$(DRVDIR)/winbond_%_xmem.dat:	$(XMEMDRIVERDIR)/%_xmem.spin $(XMEM_DRIVER_COMMON)
	@$(SPIN_DAT) -DWINBOND -o $@ $<
	@$(ECHO) $@

$(DRVDIR)/sst_%_xmem.dat:	$(XMEMDRIVERDIR)/%_xmem.spin $(XMEM_DRIVER_COMMON)
	@$(SPIN_DAT) -DSST -o $@ $<
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

.PHONY:	propeller-elf-image-size
propeller-elf-image-size:		$(BINDIR)/propeller-elf-image-size$(EXT)

$(BINDIR)/propeller-elf-image-size$(EXT):	$(BINDIR) $(OBJDIR) $(IMAGE_SIZE_OBJS)
	@$(CC) $(LDFLAGS) -o $@ $(IMAGE_SIZE_OBJS)
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
	$(CP) -f $(BUILDROOT)/bin/$(OS)/propeller-elf-image-size $(INSTALLBINDIR)
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
