TARGETS = eeprom-lmm.elf eeprom-xmmc.elf

BOARD ?= c3

all:	$(TARGETS)

eeprom-lmm.elf:	eepromdemo.c
	propeller-elf-gcc -Os -mlmm -o $@ eepromdemo.c

eeprom-xmmc.elf:	eepromdemo.c
	propeller-elf-gcc -Os -mxmmc -o $@ eepromdemo.c

run-lmm:
	propeller-load eeprom-lmm.elf -r -t

run-xmmc:
	propeller-load -b $(BOARD) eeprom-xmmc.elf -r -t

clean:
	rm -f $(TARGETS)

