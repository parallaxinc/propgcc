######################
# sd_loader Makefile #
######################

ifndef BUILDROOT
BUILDROOT=.
endif

SRCDIR=.
OBJDIR=$(BUILDROOT)/obj
DRVDIR=$(BUILDROOT)/include

DIRS = $(OBJDIR) $(DRVDIR)

CC=propeller-elf-gcc
ECHO=echo
MKDIR=mkdir -p
RM=rm

CFLAGS=-Os -Wall -mlmm

HDRS=sd_loader.h fatread.h sdio.h ../src/pex.h

all:	$(DRVDIR)/sd_loader.elf $(DRVDIR)/sd_cache_loader.elf

SDLOADER_OBJS=\
$(OBJDIR)/sd_loader.o \
$(OBJDIR)/fatread.o \
$(OBJDIR)/sdio.o \
$(OBJDIR)/vm_start.o

SDCACHELOADER_OBJS=\
$(OBJDIR)/sd_cache_loader.o \
$(OBJDIR)/fatread.o \
$(OBJDIR)/sdio.o \
$(OBJDIR)/vm_start.o

$(DRVDIR)/sd_loader.elf:	$(DRVDIR) $(SDLOADER_OBJS)
	propeller-elf-gcc $(CFLAGS) -o $@ $(SDLOADER_OBJS)

$(DRVDIR)/sd_cache_loader.elf:	$(DRVDIR) $(SDCACHELOADER_OBJS)
	propeller-elf-gcc $(CFLAGS) -o $@ $(SDCACHELOADER_OBJS)

#########
# RULES #
#########

$(OBJDIR)/%.o:	$(SRCDIR)/%.c $(OBJDIR) $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@
	@$(ECHO) $@

$(OBJDIR)/%.o:	$(SRCDIR)/%.S $(OBJDIR) $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@
	@$(ECHO) $@

###############
# DIRECTORIES #
###############

$(DIRS):
	$(MKDIR) $@

################
# CLEAN TARGET #
################

.PHONY:	clean
clean:
	@$(RM) -f -r $(OBJDIR)
	@$(RM) -f $(DRVDIR)/sd_loader.elf $(DRVDIR)/sd_cache_loader.elf
