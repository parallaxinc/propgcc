TARGETS = eeprom-lmm.elf eeprom-xmmc.elf

BOARD ?= c3

all:	$(TARGETS)

eeprom-lmm.elf:	eeprom.c
	propeller-elf-gcc -Os -mlmm -o $@ eeprom.c

eeprom-xmmc.elf:	eeprom.c
	propeller-elf-gcc -Os -mxmmc -o $@ eeprom.c

run-lmm:
	propeller-load eeprom-lmm.elf -r -t

run-xmmc:
	propeller-load -b $(BOARD) eeprom-xmmc.elf -r -t

clean:
	rm -f $(TARGETS)

