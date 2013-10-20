XMEM_DRIVER_COMMON = xmem_common.spin xmem_interface.spin

DRIVERS = \
winbond_spi_flash_xmem.dat \
sst_spi_flash_xmem.dat \
spi_sram_xmem.dat \
spi_sram24_xmem.dat \
sqi_sram_xmem.dat

all:	$(DRIVERS)

%.dat:	%.spin $(XMEM_DRIVER_COMMON)
	spin2cpp --dat -o $@ $<

winbond_%.dat:	%.spin $(XMEM_DRIVER_COMMON)
	spin2cpp --dat -DWINBOND -o $@ $<

sst_%.dat:	%.spin $(XMEM_DRIVER_COMMON)
	spin2cpp --dat -DSST -o $@ $<

clean:
	rm -f $(DRIVERS)
