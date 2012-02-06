MODEL = xmmc
CC = propeller-elf-gcc
CFLAGS = -m$(MODEL) -std=c99 -Os -Wall -u __serial_exit

BUILD=../../../build/lib/obj/$(MODEL)
LIB=$(BUILD)/libc.a
LIBM=$(BUILD)/libm.a $(BUILD)/libc.a

tests: strtod.run scanf.run sprintf.run strtol.run wstring.run ctype.run

ctype.elf: ctype.o $(LIB)
	$(CC) $(CFLAGS) -o $@ $^

wstring.elf: wstring.o $(LIB)
	$(CC) $(CFLAGS) -o $@ $^

strtol.elf: strtol.o $(LIB)
	$(CC) $(CFLAGS) -o $@ $^

strtod.elf: strtod.o $(LIBM)
	$(CC) $(CFLAGS) -o $@ $^

sprintf.elf: sprintf.o $(LIB)
	$(CC) $(CFLAGS) -o $@ $^

scanf.elf: scanf.o $(LIB)
	$(CC) $(CFLAGS) -o $@ $^


%.run: %.elf
	propeller-load -bc3 -p1 $^ -r -t -q

clean:
	rm -rf *.o *.elf
