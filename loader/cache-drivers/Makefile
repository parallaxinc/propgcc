CC=propeller-elf-gcc
OBJCOPY=propeller-elf-objcopy

CACHE_DRIVER_COMMON = cache_common.spin cache_interface.spin cache_spi_pins.spin cache_sqi_pins.spin cache_spi.spin cache_sqi.spin

DEMO?=fibo-xmmc.elf

TESTDRIVER?=rampage2_xcache

DRIVERS = \
rampage2_xcache.dat \
winbond_spi_flash_xcache.dat \
sst_spi_flash_xcache.dat \
winbond_sqi_flash_xcache.dat \
sst_sqi_flash_xcache.dat \
spi_sram_xcache.dat \
spi_sram24_xcache.dat \
sqi_sram_xcache.dat \
winbond_sqi_flash_sram_xcache.dat \
sst_sqi_flash_sram_xcache.dat \
c3_xcache.dat \
synapse_xcache.dat

.PHONY:	all
all:	$(DRIVERS)

%.dat:	%.spin $(CACHE_DRIVER_COMMON)
	spin2cpp --dat -o $@ $<

winbond_spi_flash_xcache.dat:	spi_flash_xcache.spin $(CACHE_DRIVER_COMMON)
	spin2cpp --dat -DWINBOND -o $@ $<

sst_spi_flash_xcache.dat:	spi_flash_xcache.spin $(CACHE_DRIVER_COMMON)
	spin2cpp --dat -DSST -o $@ $<

winbond_sqi_flash_xcache.dat:	sqi_flash_xcache.spin $(CACHE_DRIVER_COMMON)
	spin2cpp --dat -DWINBOND -o $@ $<

sst_sqi_flash_xcache.dat:	sqi_flash_xcache.spin $(CACHE_DRIVER_COMMON)
	spin2cpp --dat -DSST -o $@ $<

winbond_sqi_flash_sram_xcache.dat:	sqi_flash_sram_xcache.spin $(CACHE_DRIVER_COMMON)
	spin2cpp --dat -DWINBOND -o $@ $<

sst_sqi_flash_sram_xcache.dat:	sqi_flash_sram_xcache.spin $(CACHE_DRIVER_COMMON)
	spin2cpp --dat -DSST -o $@ $<

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

.PHONY:	cachetest
cachetest:	cachetest.elf

cachetest.elf:	cachetest.c $(TESTDRIVER).o
	$(CC) -Wall -D TESTDRIVER=binary_$(TESTDRIVER)_dat_start -mcmm -Os -mno-fcache -o $@ $^
    
%.o: %.dat
	$(OBJCOPY) -I binary -B propeller -O $(CC) $< $@

install:
	cp *.dat *.cfg /opt/parallax/propeller-load

clean:
	rm -f $(DRIVERS)
