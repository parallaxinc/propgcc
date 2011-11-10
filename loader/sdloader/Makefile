######################
# sd_loader Makefile #
######################

ifndef TARGET
TARGET=.
endif

SRCDIR=.
OBJDIR=$(TARGET)/obj
DRVDIR=$(TARGET)/include

DIRS = $(OBJDIR) $(DRVDIR)

CC=propeller-elf-gcc
ECHO=echo
MKDIR=mkdir -p
RM=rm

CFLAGS=-Os -Wall -mlmm

HDRS=sd_loader.h fatread.h sdio.h ../src/pex.h

OBJS=\
$(OBJDIR)/sd_loader.o \
$(OBJDIR)/fatread.o \
$(OBJDIR)/sdio.o \
$(OBJDIR)/vm_start.o

$(DRVDIR)/sd_loader.elf:	$(DRVDIR) $(OBJS)
	propeller-elf-gcc $(CFLAGS) -o $@ $(OBJS)

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
	@$(RM) -f $(DRVDIR)/sd_loader.elf
	
	
