CC=propeller-elf-gcc
OBJCOPY=propeller-elf-objcopy

#TESTDRIVER?=spi_flash_xmem
#TESTDRIVER?=spi_sram_xmem
TESTDRIVER?=sqi_sram_xmem
#TESTDRIVER?=winbond_sqi_flash_sram_xmem
#TESTDRIVER?=synapse_xmem
#TESTDRIVER?=rampage2_xmem
#TESTDRIVER?=rampage2_sram_xmem
#TESTDRIVER?=pmc_xmem
#TESTDRIVER?=c3_xmem

XMEM_DRIVER_COMMON = xmem_common.spin

.PHONY:	all
all:	xmemtest.elf $(TESTDRIVER).dat

%.dat:	%.spin $(XMEM_DRIVER_COMMON)
	spin2cpp --dat -o $@ $<

winbond_%.dat:	%.spin $(XMEM_DRIVER_COMMON)
	spin2cpp --dat -D WINBOND -o $@ $<

sst_%.dat:	%.spin $(XMEM_DRIVER_COMMON)
	spin2cpp --dat -D SST -o $@ $<

%.o: %.dat
	$(OBJCOPY) -I binary -B propeller -O $(CC) $< $@

xmemtest.elf:	xmemtest.c $(TESTDRIVER).o
	$(CC) -Wall -D TESTDRIVER=binary_$(TESTDRIVER)_dat_start -mcmm -mno-fcache -o $@ $^
    
run:	xmemtest.elf
	propeller-load xmemtest.elf -r -t

clean:
	rm -f *.o $(TESTDRIVER).dat xmemtest.elf
