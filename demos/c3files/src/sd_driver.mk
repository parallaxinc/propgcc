
BSTC=bstc
ECHO=echo
SPINFLAGS=-Ogxr
OBJDIR=./
SPINDIR=.
SPIN_SRCS=.
SPIN_SRCS=$(SPINDIR)/sd_driver.spin

sd_driver.bin:	sd_driver.spin
	$(BSTC) $(SPINFLAGS) -c -o sd_driver sd_driver.spin
	./bin2c sd_driver.dat sd_driver.c

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

