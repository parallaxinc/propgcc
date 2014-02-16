######################
# sd_loader Makefile #
######################

ifndef BUILDROOT
BUILDROOT=.
endif

SRCDIR=.
OBJDIR=$(BUILDROOT)/obj
DRVDIR=$(BUILDROOT)/propeller-load

CC=propeller-elf-gcc
ECHO=echo
MKDIR=mkdir -p
RM=rm
TOUCH=touch

CFLAGS=-Os -Wall -mcmm
LFLAGS=-Wl,--defsym -Wl,__stack_end=0x5000

HDRS=sd_loader.h fatread.h sdio.h ../src/pex.h

all:	$(DRVDIR)/sd_loader.elf $(DRVDIR)/sd_cache_loader.elf

SDLOADER_OBJS=\
$(OBJDIR)/sd_loader.o \
$(OBJDIR)/fatread.o \
$(OBJDIR)/sdio.o

SDCACHELOADER_OBJS=\
$(OBJDIR)/sd_cache_loader.o \
$(OBJDIR)/fatread.o \
$(OBJDIR)/sdio.o \
$(OBJDIR)/vm_start.o

$(DRVDIR)/sd_loader.elf:	$(DRVDIR)/dir-created $(SDLOADER_OBJS)
	propeller-elf-gcc $(CFLAGS) $(LFLAGS) -o $@ $(SDLOADER_OBJS)

$(DRVDIR)/sd_cache_loader.elf:	$(DRVDIR)/dir-created $(SDCACHELOADER_OBJS)
	propeller-elf-gcc $(CFLAGS) -o $@ $(SDCACHELOADER_OBJS)

#########
# RULES #
#########

$(OBJDIR)/%.o:	$(SRCDIR)/%.c $(OBJDIR)/dir-created $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@
	@$(ECHO) $@

$(OBJDIR)/%.o:	$(SRCDIR)/%.S $(OBJDIR)/dir-created $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@
	@$(ECHO) $@

###############
# DIRECTORIES #
###############

%/dir-created:
	@$(MKDIR) -p $(@D)
	@$(TOUCH) $@

################
# CLEAN TARGET #
################

.PHONY:	clean
clean:
	@$(RM) -f -r $(OBJDIR)
	@$(RM) -f $(DRVDIR)/sd_loader.elf $(DRVDIR)/sd_cache_loader.elf
