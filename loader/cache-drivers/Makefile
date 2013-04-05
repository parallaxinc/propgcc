CACHE_DRIVER_COMMON = cache_common.spin cache_interface.spin

DEMO?=fibo.elf

DRIVERS = \
rampage2_xcache.dat \
spi_flash_xcache.dat \
sqi_flash_xcache.dat \
spi_sram_xcache.dat \
spi_sram24_xcache.dat \
sqi_sram_xcache.dat \
c3_xcache.dat

all:	$(DRIVERS)

%.dat:	%.spin $(CACHE_DRIVER_COMMON)
	spin2cpp --dat -o $@ $<

run rampage2x:	rampage2_xcache.dat
	propeller-load -b rampage2x $(DEMO) -r -t

rampage2:
	propeller-load -b rampage2 $(DEMO) -r -t

dnax:	spi_flash_xcache.dat
	propeller-load -b dnax $(DEMO) -r -t

dnax-ram:	spi_flash_xcache.dat
	propeller-load -b dnax:ram $(DEMO) -r -t

c3x:	c3_xcache.dat
	propeller-load -b c3x $(DEMO) -r -t

clean:
	rm -f $(DRIVERS)
