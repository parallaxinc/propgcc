MODEL = -mxmmc
CC = propeller-elf-gcc
CFLAGS = $(MODEL) -Os -Wall -u __serial_exit

tests: ctype.run

ctype.elf: ctype.o
	$(CC) $(CFLAGS) -o $@ $^

ctype.run: ctype.elf
	propeller-load -bc3 -p1 $^ -r -t -q
