#PREFIX?=/opt/parallax
PREFIX?=$(shell pwd)/target

ROOT=$(shell pwd)
BUILD=../build

ECHO=echo
RM=rm
CD=cd
MKDIR=mkdir -p
CHMOD=chmod
CP=cp

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
  $(error "Unknown system: " $(UNAME))
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
binutils:	$(BUILD)/binutils
	@$(ECHO) Building binutils
	@$(MAKE) -C $< all
	@$(ECHO) Installing binutils
	@$(MAKE) -C $< install
	
$(BUILD)/binutils:
	@$(ECHO) Configuring binutils
	@$(MKDIR) -p $@
	@$(CD) $@; $(ROOT)/binutils/configure --target=propeller-elf --prefix=$(PREFIX) --disable-nls --disable-shared

#######
# GCC #
#######

.PHONY:	gcc
gcc:	$(BUILD)/gcc
	@$(ECHO) Building gcc
	@$(MAKE) -C $< all-gcc
	@$(ECHO) Installing gcc
	@$(MAKE) -C $< install-gcc
	
$(BUILD)/gcc:
	@$(ECHO) Configuring gcc
	$(MKDIR) -p $@
	$(CD) $@; $(ROOT)/gcc/configure --target=propeller-elf --prefix=$(PREFIX) --disable-nls --disable-shared

#############
# LIBSTDC++ #
#############

.PHONY:	libstdc++
libstdc++:	$(BUILD)/gcc
	@$(ECHO) Building libstdc++
	@$(MAKE) -C $< all
	@$(ECHO) Installing libstdc++
	@$(MAKE) -C $< install
	
#######
# GDB #
#######

.PHONY:	gdb
gdb:	$(BUILD)/gdb
	@$(ECHO) Building gdb
	@$(MAKE) -C $< all-gcc
	@$(ECHO) Installing gdb
	@$(MAKE) -C $< install-gcc
	@$(ECHO) Installing gdb
	@$(CP) -f gdb/gdb$(EXT) ${PREFIX}/bin/propeller-elf-gdb$(EXT)
	
$(BUILD)/gdb:
	@$(ECHO) Configuring gdb
	$(MKDIR) -p $@
	$(CD) $@; $(ROOT)/gdb/configure --target=propeller-elf --prefix=$(PREFIX) --with-system-gdbinit=${PREFIX}/lib/gdb/gdbinit

###########
# GDBSTUB #
###########

.PHONY:	gdbstub
gdbstub:
	@$(ECHO) Building gdbstub
	@$(MAKE) -C gdbstub
	@$(ECHO) Installing gdbstub
	@$(CP) -f gdbstub/gdbstub$(EXT) $(PREFIX)/bin/
	@$(MKDIR) -p $(PREFIX)/lib/gdb
	@$(CP) -f gdbstub/gdbinit.propeller $(PREFIX)/lib/gdb/gdbinit

##########
# LIBGCC #
##########

.PHONY:	libgcc
libgcc:	$(BUILD)/gcc
	@$(ECHO) Building libgcc
	@$(MAKE) -C $< all-target-libgcc
	@$(ECHO) Installing gcc
	@$(MAKE) -C $< install-target-libgcc
	
$(BUILD)/gcc:
	@$(ECHO) Configuring gcc
	@$(MKDIR) -p $@
	@$(CD) $@; $(ROOT)/gcc/configure --target=propeller-elf --prefix=$(PREFIX) --disable-nls --disable-shared

###############
# COG LIBRARY #
###############

.PHONY:	lib-cog
lib-cog:
	@$(ECHO) Building cog library
	@$(MAKE) -C lib cog

###########
# LIBRARY #
###########

.PHONY:	lib
lib:
	@$(ECHO) Building library
	@$(MAKE) -C lib
	@$(ECHO) Installing library
	@$(MAKE) -C lib install

###########
# LIBTINY #
###########

.PHONY:	lib-tiny
lib-tiny:
	@$(ECHO) Building tiny library
	@$(MAKE) -C lib tiny
	@$(ECHO) Installing tiny library
	@$(MAKE) -C lib install-tiny

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
spin2cpp:
	@$(ECHO) Building spin2cpp
	@$(MAKE) -C spin2cpp TARGET=$(PREFIX) BUILDROOT=$(BUILD)/spin2cpp
	@$(ECHO) Installing spin2cpp
	@$(MAKE) -C spin2cpp TARGET=$(PREFIX) BUILDROOT=$(BUILD)/spin2cpp install

###########
# SPINSIM #
###########

.PHONY:	spinsim
spinsim:
	@$(ECHO) Building spinsim
	@$(MAKE) -C spinsim
	@$(CP) -f spinsim/spinsim$(EXT) ${PREFIX}/bin/

##########
# LOADER #
##########

.PHONY:	loader
loader:
	@$(ECHO) Building propeller-load
	@$(MAKE) -C loader TARGET=$(PREFIX) BUILDROOT=$(BUILD)/loader
	@$(ECHO) Installing propeller-load
	@$(MAKE) -C loader TARGET=$(PREFIX) BUILDROOT=$(BUILD)/loader install

#########
# CLEAN #
#########

.PHONY:	clean
clean:
	@$(ECHO) Removing $(BUILD).
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
	@$(ECHO) Removing $(PREFIX).
	@$(RM) -rf $(PREFIX)
