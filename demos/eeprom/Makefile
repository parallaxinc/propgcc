TARGET = eeprom.elf

$(TARGET):	eeprom.c
	propeller-elf-gcc -Os -mlmm -o $@ eeprom.c

run:
	propeller-load $(TARGET) -r -t

clean:
	rm -f $(TARGET)

