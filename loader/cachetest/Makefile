CC=propeller-elf-gcc
OBJCOPY=propeller-elf-objcopy

//TESTDRIVER?=c3_xcache
TESTDRIVER?=winbond_sqi_flash_sram_xcache
//TESTDRIVER?=rampage2_xcache

CACHE_DRIVER_COMMON = cache_interface.spin cache_common.spin cache_sqi.spin

.PHONY:	all
all:	cachetest.elf $(TESTDRIVER).dat

%.dat:	%.spin $(CACHE_DRIVER_COMMON)
	spin2cpp --dat -o $@ $<

winbond_sqi_flash_sram_xcache.dat:	sqi_flash_sram_xcache.spin $(CACHE_DRIVER_COMMON)
	spin2cpp --dat -DWINBOND -o $@ $<

%.o: %.dat
	$(OBJCOPY) -I binary -B propeller -O $(CC) $< $@

cachetest.elf:	cachetest.c $(TESTDRIVER).o
	$(CC) -Wall -D TESTDRIVER=binary_$(TESTDRIVER)_dat_start -mcmm -mno-fcache -o $@ $^
    
run:	cachetest.elf
	propeller-load cachetest.elf -r -t

zip:	clean
	rm -f cachetest.zip
	zip cachetest cachetest.c *.spin Makefile

clean:
	rm -f *.o $(TESTDRIVER).dat cachetest.elf
