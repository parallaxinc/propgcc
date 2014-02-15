PREFIX?=/opt/parallax
#PREFIX?=$(shell pwd)/target

ROOT=$(shell pwd)
BUILD=../build

ECHO=echo
RM=rm
CD=cd
MKDIR=mkdir -p
CHMOD=chmod
CP=cp
TOUCH=touch

UNAME=$(shell uname -s)

ifeq ($(UNAME),Linux)
  OS=linux
  SPINCMP=openspin.linux
  EXT=
endif

ifeq ($(UNAME),Darwin)
  OS=macosx
  SPINCMP=openspin.osx
  EXT=
endif

ifeq ($(UNAME),Msys)
  OS=msys
  SPINCMP=openspin.exe
  EXT=.exe
endif

ifeq ($(OS),)
  $(error Unknown system: $(UNAME))
endif

export PREFIX
export OS

.PHONY:	all
all:	binutils gcc lib-cog libgcc lib install-spin-compiler lib-tiny spin2cpp loader gdb gdbstub spinsim libstdc++
	@$(ECHO) Build complete.

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
	@$(CD) $(BUILD)/binutils; $(ROOT)/binutils/configure --target=propeller-elf --prefix=$(PREFIX) --disable-nls --disable-shared
	@$(TOUCH) $@

#######
# GCC #
#######

.PHONY:	gcc
gcc:	$(BUILD)/gcc/gcc-built

$(BUILD)/gcc/gcc-built:	$(BUILD)/binutils/binutils-built $(BUILD)/gcc/gcc-configured
	@$(ECHO) Building gcc
	@$(MAKE) -C $(BUILD)/gcc all-gcc
	@$(ECHO) Installing gcc
	@$(MAKE) -C $(BUILD)/gcc install-gcc
	@$(TOUCH) $@
	
$(BUILD)/gcc/gcc-configured:	$(BUILD)/gcc/gcc-created
	@$(ECHO) Configuring gcc
	@$(CD) $(BUILD)/gcc; $(ROOT)/gcc/configure --target=propeller-elf --prefix=$(PREFIX) --disable-nls --disable-shared
	@$(TOUCH) $@

#############
# LIBSTDC++ #
#############

.PHONY:	libstdc++
libstdc++:	$(BUILD)/gcc/libstdc++-built

$(BUILD)/gcc/libstdc++-built:	$(BUILD)/gcc/gcc-built
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

$(BUILD)/gcc/libgcc-built:	$(BUILD)/gcc/gcc-built
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
	@$(CD) $(BUILD)/gdb; $(ROOT)/gdb/configure --target=propeller-elf --prefix=$(PREFIX) --with-system-gdbinit=$(PREFIX)/lib/gdb/gdbinit
	@$(TOUCH) $@

###########
# GDBSTUB #
###########

.PHONY:	gdbstub
gdbstub:	$(BUILD)/gdb/gdb-built

$(BUILD)/gdbstub/gdbstub-built:	$(BUILD)/gdbstub/gdbstub-created
	@$(ECHO) Building gdbstub
	@$(MAKE) -C gdbstub
	@$(ECHO) Installing gdbstub
	@$(CP) -f gdbstub/gdbstub$(EXT) $(PREFIX)/bin/
	@$(MKDIR) -p $(PREFIX)/lib/gdb
	@$(CP) -f gdbstub/gdbinit.propeller $(PREFIX)/lib/gdb/gdbinit
	@$(TOUCH) $@

#######
# LIB #
#######

.PHONY:	lib
lib:	$(BUILD)/lib/lib-built

$(BUILD)/lib/lib-built:	$(BUILD)/lib/lib-created
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

$(BUILD)/lib/lib-cog-built:	$(BUILD)/lib/lib-created
	@$(ECHO) Building cog library
	@$(MAKE) -C lib cog
	@$(TOUCH) $@

###########
# LIBTINY #
###########

.PHONY:	lib-tiny
lib-tiny:	$(BUILD)/lib/lib-tiny-built

$(BUILD)/lib/lib-tiny-built:	$(BUILD)/lib/lib-created
	@$(ECHO) Building tiny library
	@$(MAKE) -C lib tiny
	@$(ECHO) Installing tiny library
	@$(MAKE) -C lib install-tiny
	@$(TOUCH) $@

#################
# SPIN COMPILER #
#################

.PHONY:	install-spin-compiler
install-spin-compiler:
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
	@$(MAKE) -C spinsim
	@$(CP) -f spinsim/spinsim$(EXT) $(PREFIX)/bin/
	@$(TOUCH) $@

##########
# LOADER #
##########

.PHONY:	loader
loader:	$(BUILD)/loader/loader-built

$(BUILD)/loader/loader-built:	$(BUILD)/loader/loader-created
	@$(ECHO) Building propeller-load
	@$(MAKE) -C loader TARGET=$(PREFIX) BUILDROOT=$(BUILD)/loader
	@$(ECHO) Installing propeller-load
	@$(MAKE) -C loader TARGET=$(PREFIX) BUILDROOT=$(BUILD)/loader install
	@$(TOUCH) $@

#########
# CLEAN #
#########

.PHONY:	clean
clean:
	@$(ECHO) Removing $(BUILD)
	@$(RM) -rf $(BUILD)
	@$(MAKE) -C lib clean
	@$(MAKE) -C spin2cpp clean
	@$(MAKE) -C loader clean
	@$(MAKE) -C gdbstub clean
	@$(MAKE) -C spinsim clean

#############
# CLEAN-ALL #
#############

.PHONY:	clean-all
clean-all:	clean
	@$(ECHO) Removing $(PREFIX)/*
	@$(RM) -rf $(PREFIX)/*

# create a directory
	
$(BUILD)/%-created:
	@$(MKDIR) -p $(@D)
	@$(TOUCH) $@


