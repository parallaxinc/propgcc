CC=propeller-elf-gcc
OBJCOPY=propeller-elf-objcopy

//TESTDRIVER?=c3_xcache
//TESTDRIVER?=winbond_sqi_flash_sram_xcache
//TESTDRIVER?=sst_sqi_flash_sram_xcache
TESTDRIVER?=sqi_sram_xcache
//TESTDRIVER?=spi_sram24_xcache
//TESTDRIVER?=rampage2_xcache

//TESTBOARD?=PMC_PROTOTYPE
//TESTBOARD?=PMC
TESTBOARD?=PMC_SQI_SRAM
//TESTBOARD?=PMC_SPI_SRAM24
//TESTBOARD?=RAMPAGE2_PMC
//TESTBOARD?=RAMPAGE2
//TESTBOARD?=RAMPAGE2_SQI_SRAM
//TESTBOARD?=RAMPAGE2_SPI_SRAM24

CACHE_DRIVER_COMMON=\
../cache-drivers/cache_interface.spin \
../cache-drivers/cache_common.spin \
../cache-drivers/cache_sqi_pins.spin \
../cache-drivers/cache_sqi.spin \
../cache-drivers/cache_spi_pins.spin \
../cache-drivers/cache_spi.spin

.PHONY:	all
all:	cachetest.elf $(TESTDRIVER).dat

%.dat:	../cache-drivers/%.spin $(CACHE_DRIVER_COMMON)
	spin2cpp --dat -o $@ $<

winbond_%.dat:	../cache-drivers/%.spin $(CACHE_DRIVER_COMMON)
	spin2cpp --dat -DWINBOND -o $@ $<

sst_%.dat:	../cache-drivers/%.spin $(CACHE_DRIVER_COMMON)
	spin2cpp --dat -DSST -o $@ $<

%.o: %.dat
	$(OBJCOPY) -I binary -B propeller -O $(CC) $< $@

cachetest.elf:	cachetest.c $(TESTDRIVER).o Makefile
	$(CC) -Wall -D TESTDRIVER=binary_$(TESTDRIVER)_dat_start -D $(TESTBOARD) -mcmm -mno-fcache -o $@ cachetest.c $(TESTDRIVER).o
    
run:	cachetest.elf
	propeller-load cachetest.elf -r -t

clean:
	rm -f *.o $(TESTDRIVER).dat cachetest.elf
