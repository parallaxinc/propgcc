CC=propeller-elf-gcc
OBJCOPY=propeller-elf-objcopy

//TESTDRIVER?=c3_xcache
TESTDRIVER?=winbond_sqi_flash_sram_xcache
//TESTDRIVER?=sst_sqi_flash_sram_xcache
//TESTDRIVER?=rampage2_xcache

//TESTBOARD?=PMC_PROTOTYPE
TESTBOARD?=PMC
//TESTBOARD?=RAMPAGE2_PMC
//TESTBOARD?=RAMPAGE2

CACHE_DRIVER_COMMON=\
../cache-drivers/cache_interface.spin \
../cache-drivers/cache_common.spin \
../cache-drivers/cache_sqi.spin \
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

cachetest.elf:	cachetest.c $(TESTDRIVER).o
	$(CC) -Wall -D TESTDRIVER=binary_$(TESTDRIVER)_dat_start -D $(TESTBOARD) -mcmm -mno-fcache -o $@ $^
    
run:	cachetest.elf
	propeller-load cachetest.elf -r -t

clean:
	rm -f *.o $(TESTDRIVER).dat cachetest.elf
