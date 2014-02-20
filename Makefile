#
# Normally this Makefile builds binaries for the same system as the host,
# i.e. when run on linux x86 it produces a linux x86 propgcc toolchain.
# However, you can also generate a toolchain for a different platform. To
# do this, first make for the host (just do a plain "make") and then do a
# "make CROSS=win32" to build a win32 toolchain.
#
ROOT=$(shell pwd)
ifeq ($(CROSS),)
  BUILD=$(ROOT)/../build
  CFGCROSS=
  CROSSCC=gcc
else
  BUILD=$(ROOT)/../build-$(CROSS)
  PREFIX=/opt/parallax-$(CROSS)
  CFGCROSS=--host=i586-mingw32msvc
  CROSSCC=i586-mingw32msvc-gcc
  OS=msys
  EXT=.exe
endif

PREFIX?=/opt/parallax

ECHO=echo
RM=rm
CD=cd
MKDIR=mkdir -p
CHMOD=chmod
CP=cp
TOUCH=touch

UNAME=$(shell uname -s)

ifeq ($(UNAME),Linux)
  OS?=linux
  SPINCMP=openspin.linux
  EXT?=
endif

ifeq ($(UNAME),Darwin)
  OS?=macosx
  SPINCMP=openspin.osx
  EXT?=
endif

ifeq ($(UNAME),Msys)
  OS?=msys
  SPINCMP=openspin.exe
  EXT?=.exe
endif

ifeq ($(OS),)
  $(error Unknown system: $(UNAME))
endif

$(warning OS $(OS) detected.)

export PREFIX
export OS
export SPINCMP

#
# note that the propgcc version string does not deal well with
# spaces due to how it is used below
#
VERSION=$(shell cat release/VERSION.txt | grep -v '^\#')
# better revision command. thanks yeti.
HGVERSION=$(shell hg tip --template '{rev}\n')

PROPGCC_VERSION=$(VERSION)_$(HGVERSION)

$(warning PropGCC version is $(PROPGCC_VERSION).)
export PROPGCC_VERSION

BUGURL?=http://code.google.com/p/propgcc/issues
$(warning BugURL is $(BUGURL).)
export BUGURL

#
# configure options for propgcc
#
CONFIG_OPTIONS=--with-pkgversion=$(PROPGCC_VERSION) --with-bugurl=$(BUGURL) $(CFGCROSS)

.PHONY:	all
all:	binutils gcc lib-cog libgcc lib install-spin-compiler lib-tiny spin2cpp loader gdb gdbstub spinsim libstdc++
	@$(ECHO) Build complete.

########
# HELP #
########

.PHONY:	help
help:
	@$(ECHO)
	@$(ECHO) 'Targets:'
	@$(ECHO) '  all - build all targets (default)'
	@$(ECHO) '  binutils - build binutils'	
	@$(ECHO) '  gcc - build gcc'	
	@$(ECHO) '  libstdc++ - build the C++ library'	
	@$(ECHO) '  libgcc - build libgcc'	
	@$(ECHO) '  gdb - build gdb'	
	@$(ECHO) '  gdbstub - build gdbstub'	
	@$(ECHO) '  lib - build the library'	
	@$(ECHO) '  lib-cog - build the cog library'	
	@$(ECHO) '  lib-tiny - build libtiny'	
	@$(ECHO) '  install-spin-compiler - install OpenSpin'	
	@$(ECHO) '  spin2cpp - build spin2cpp'	
	@$(ECHO) '  spinsim - build spinsim'	
	@$(ECHO) '  loader - build the loader'	
	@$(ECHO)
	@$(ECHO) 'Cleaning targets:'
	@$(ECHO) '  clean - prepare for a fresh build'
	@$(ECHO) '  clean-all - prepare for a fresh build and remove the $(PREFIX) directory'
	@$(ECHO) '  clean-binutils - prepare for a fresh rebuild of binutils'	
	@$(ECHO) '  clean-gcc - prepare for a fresh rebuild of gcc, libgcc, libstdc++'	
	@$(ECHO) '  clean-gdb - prepare for a fresh rebuild of gdb'	
	@$(ECHO) '  clean-gdbstub - prepare for a fresh rebuild of gdbstub'	
	@$(ECHO) '  clean-lib - prepare for a fresh rebuild of lib, lib-cog, lib-tiny'	
	@$(ECHO) '  clean-spin2cpp - prepare for a fresh rebuild of spin2cpp'	
	@$(ECHO) '  clean-spinsim prepare for a fresh rebuild of spinsim'	
	@$(ECHO)

############
# BINUTILS #
############

.PHONY:	binutils
binutils:	$(BUILD)/binutils/binutils-built

$(BUILD)/binutils/binutils-built:	$(BUILD)/binutils/binutils-configured
	@$(ECHO) Building binutils
	@$(MAKE) -C $(BUILD)/binutils all
	@$(ECHO) Installing binutils
	@$(MAKE) -C $(BUILD)/binutils install
	@$(TOUCH) $@

$(BUILD)/binutils/binutils-configured:	$(BUILD)/binutils/binutils-created
	@$(ECHO) Configuring binutils
	@$(CD) $(BUILD)/binutils; $(ROOT)/binutils/configure --target=propeller-elf --prefix=$(PREFIX) --disable-nls --disable-shared $(CONFIG_OPTIONS)
	@$(TOUCH) $@

#######
# GCC #
#######

.PHONY:	gcc
gcc:	$(BUILD)/gcc/gcc-built

$(BUILD)/gcc/gcc-built:	binutils $(BUILD)/binutils/binutils-built $(BUILD)/gcc/gcc-configured
	@$(ECHO) Building gcc
	@$(MAKE) -C $(BUILD)/gcc all-gcc
	@$(ECHO) Installing gcc
	@$(MAKE) -C $(BUILD)/gcc install-gcc
	@$(TOUCH) $@

$(BUILD)/gcc/gcc-configured:	$(BUILD)/gcc/gcc-created
	@$(ECHO) Configuring gcc
	@$(CD) $(BUILD)/gcc; $(ROOT)/gcc/configure --target=propeller-elf --prefix=$(PREFIX) --disable-nls --disable-shared $(CONFIG_OPTIONS)
	@$(TOUCH) $@

#############
# LIBSTDC++ #
#############

.PHONY:	libstdc++
libstdc++:	$(BUILD)/gcc/libstdc++-built

$(BUILD)/gcc/libstdc++-built:	gcc $(BUILD)/gcc/gcc-built
	@$(ECHO) Building libstdc++
	@$(MAKE) -C $(BUILD)/gcc all
	@$(ECHO) Installing libstdc++
	@$(MAKE) -C $(BUILD)/gcc install
	@$(TOUCH) $@

##########
# LIBGCC #
##########

.PHONY:	libgcc
libgcc:	$(BUILD)/gcc/libgcc-built

$(BUILD)/gcc/libgcc-built:	gcc $(BUILD)/gcc/gcc-built
	@$(ECHO) Building libgcc
	@$(MAKE) -C $(BUILD)/gcc all-target-libgcc
	@$(ECHO) Installing gcc
	@$(MAKE) -C $(BUILD)/gcc install-target-libgcc
	@$(TOUCH) $@

#######
# GDB #
#######

.PHONY:	gdb
gdb:	$(BUILD)/gdb/gdb-built

$(BUILD)/gdb/gdb-built:	$(BUILD)/gdb/gdb-configured
	@$(ECHO) Building gdb
	@$(MAKE) -C $(BUILD)/gdb all
	@$(ECHO) Installing gdb
	@$(CP) -f $(BUILD)/gdb/gdb/gdb$(EXT) $(PREFIX)/bin/propeller-elf-gdb$(EXT)
	@$(TOUCH) $@

$(BUILD)/gdb/gdb-configured:	$(BUILD)/gdb/gdb-created
	@$(ECHO) Configuring gdb
	@$(CD) $(BUILD)/gdb; $(ROOT)/gdb/configure $(CFGCROSS) --target=propeller-elf --prefix=$(PREFIX) --with-system-gdbinit=$(PREFIX)/lib/gdb/gdbinit 
	@$(TOUCH) $@

###########
# GDBSTUB #
###########

.PHONY:	gdbstub
gdbstub:	$(BUILD)/gdbstub/gdbstub-built

$(BUILD)/gdbstub/gdbstub-built:	$(BUILD)/gdbstub/gdbstub-created
	@$(ECHO) Building gdbstub
	@$(MAKE) -C gdbstub BUILDROOT=$(BUILD)/gdbstub CC=$(CROSSCC)
	@$(ECHO) Installing gdbstub
	@$(CP) -f $(BUILD)/gdbstub/gdbstub$(EXT) $(PREFIX)/bin/
	@$(MKDIR) -p $(PREFIX)/lib/gdb
	@$(CP) -f gdbstub/gdbinit.propeller $(PREFIX)/lib/gdb/gdbinit
	@$(TOUCH) $@

#######
# LIB #
#######

.PHONY:	lib
lib:	$(BUILD)/lib/lib-built

$(BUILD)/lib/lib-built:	gcc $(BUILD)/lib/lib-created
	@$(ECHO) Building library
	@$(MAKE) -C lib
	@$(ECHO) Installing library
	@$(MAKE) -C lib install
	@$(TOUCH) $@

###############
# COG LIBRARY #
###############

.PHONY:	lib-cog
lib-cog:	$(BUILD)/lib/lib-cog-built

$(BUILD)/lib/lib-cog-built:	gcc $(BUILD)/lib/lib-created
	@$(ECHO) Building cog library
	@$(MAKE) -C lib cog
	@$(TOUCH) $@

###########
# LIBTINY #
###########

.PHONY:	lib-tiny
lib-tiny:	$(BUILD)/lib/lib-tiny-built

$(BUILD)/lib/lib-tiny-built:	gcc $(BUILD)/lib/lib-created
	@$(ECHO) Building tiny library
	@$(MAKE) -C lib tiny
	@$(ECHO) Installing tiny library
	@$(MAKE) -C lib install-tiny
	@$(TOUCH) $@

#################
# SPIN COMPILER #
#################

.PHONY:	install-spin-compiler
install-spin-compiler:	$(PREFIX)/bin/bin-created
	@$(CP) -f release/$(SPINCMP) $(PREFIX)/bin
	@$(CHMOD) a+x $(PREFIX)/bin/$(SPINCMP)

############
# SPIN2CPP #
############

.PHONY:	spin2cpp
spin2cpp:	$(BUILD)/spin2cpp/spin2cpp-built

$(BUILD)/spin2cpp/spin2cpp-built:	$(BUILD)/spin2cpp/spin2cpp-created
	@$(ECHO) Building spin2cpp
	@$(MAKE) -C spin2cpp TARGET=$(PREFIX) BUILDROOT=$(BUILD)/spin2cpp
	@$(ECHO) Installing spin2cpp
	@$(MAKE) -C spin2cpp TARGET=$(PREFIX) BUILDROOT=$(BUILD)/spin2cpp install
	@$(TOUCH) $@

###########
# SPINSIM #
###########

.PHONY:	spinsim
spinsim:	$(BUILD)/spinsim/spinsim-built

$(BUILD)/spinsim/spinsim-built:	$(BUILD)/spinsim/spinsim-created
	@$(ECHO) Building spinsim
	@$(MAKE) -C spinsim CC=$(CROSSCC) OS=$(OS) BUILD=$(BUILD)/spinsim EXT=$(EXT)
	@$(CP) -f spinsim/spinsim$(EXT) $(PREFIX)/bin/
	@$(TOUCH) $@

##########
# LOADER #
##########

.PHONY:	loader
loader:	$(BUILD)/loader/loader-built

$(BUILD)/loader/loader-built:	gcc $(BUILD)/loader/loader-created
	@$(ECHO) Building propeller-load
	@$(MAKE) -C loader TARGET=$(PREFIX) BUILDROOT=$(BUILD)/loader TOOLCC=$(CROSSCC)
	@$(ECHO) Installing propeller-load
	@$(MAKE) -C loader TARGET=$(PREFIX) BUILDROOT=$(BUILD)/loader TOOLCC=$(CROSSCC) install
	@$(TOUCH) $@

#########
# CLEAN #
#########

.PHONY:	clean
clean:	clean-gdbstub clean-lib clean-loader clean-spin2cpp clean-spinsim
	@$(ECHO) Removing $(BUILD)
	@$(RM) -rf $(BUILD)

#############
# CLEAN-ALL #
#############

.PHONY:	clean-all
clean-all:	clean
	@$(ECHO) Removing $(PREFIX)/*
	@$(RM) -rf $(PREFIX)/*

#####################
# INDIVIDUAL CLEANS #
#####################

.PHONY:	clean-binutils
clean-binutils:
	@$(RM) -rf $(BUILD)/binutils

.PHONY:	clean-gcc
clean-gcc:
	@$(RM) -rf $(BUILD)/gcc

.PHONY:	clean-gdb
clean-gdb:
	@$(RM) -rf $(BUILD)/gdb

.PHONY:	clean-gdbstub
clean-gdbstub:
	@$(RM) -rf $(BUILD)/gdbstub
	@$(MAKE) -C gdbstub clean

.PHONY:	clean-lib
clean-lib:
	@$(RM) -rf $(BUILD)/lib
	@$(MAKE) -C lib clean

.PHONY:	clean-loader
clean-loader:
	@$(RM) -rf $(BUILD)/loader
	@$(MAKE) -C loader clean

.PHONY:	clean-spin2cpp
clean-spin2cpp:
	@$(RM) -rf $(BUILD)/spin2cpp
	@$(MAKE) -C spin2cpp clean

.PHONY:	clean-spinsim
clean-spinsim:
	@$(RM) -rf $(BUILD)/spinsim
	@$(MAKE) -C spinsim clean

# create a directory

%-created:
	@$(MKDIR) -p $(@D)
	@$(TOUCH) $@

