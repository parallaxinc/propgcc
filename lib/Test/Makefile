MODEL = -mxmmc
CC = propeller-elf-gcc
CFLAGS = $(MODEL) -Os -Wall -u __serial_exit

tests: strtol.run wstring.run ctype.run

ctype.elf: ctype.o
	$(CC) $(CFLAGS) -o $@ $^

wstring.elf: wstring.o
	$(CC) $(CFLAGS) -o $@ $^

strtol.elf: strtol.o
	$(CC) $(CFLAGS) -o $@ $^


%.run: %.elf
	propeller-load -bc3 -p1 $^ -r -t -q

clean:
	rm -rf *.o *.elf
