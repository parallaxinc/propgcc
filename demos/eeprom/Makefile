TARGETS=eeprom-lmm.elf eeprom-xmmc.elf

all:	$(TARGETS)

eeprom-lmm.elf:	eeprom.c
	propeller-elf-gcc -Os -mlmm -o $@ eeprom.c

eeprom-xmmc.elf:	eeprom.c
	propeller-elf-gcc -Os -mxmmc -o $@ eeprom.c

clean:
	rm -f $(TARGETS)

