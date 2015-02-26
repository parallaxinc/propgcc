MODEL = xmmc

CC = propeller-elf-gcc
CFLAGS = -m$(MODEL) -std=c99 -Os -Wall -u __serial_exit
LMMCFLAGS = -mlmm -std=c99 -Os -Wall -u __serial_exit

BUILD=../../../build/lib/obj/$(MODEL)
LIB=$(BUILD)/libc.a
LIBM=$(BUILD)/libm.a $(BUILD)/libc.a
LIBPTHREAD=$(BUILD)/libpthread.a $(BUILD)/libc.a
LIBLMM=../../../build/lib/obj/lmm/libc.a

tests: time.run strtod.run scanf.run sprintf.run strtol.run wstring.run ctype.run

#
# note: the timing functions in time.elf are quite sensitive, so run them
# in lmm mode
#
time.elf: time.c $(LIBLMM)
	$(CC) $(LMMCFLAGS) -o $@ $^

rtc.elf: rtc.o $(LIB)
	$(CC) $(CFLAGS) -o $@ $^

ctype.elf: ctype.o $(LIB)
	$(CC) $(CFLAGS) -o $@ $^

wstring.elf: wstring.o $(LIB)
	$(CC) $(CFLAGS) -o $@ $^

strtol.elf: strtol.o $(LIB)
	$(CC) $(CFLAGS) -o $@ $^

strtod.elf: strtod.o $(LIBM)
	$(CC) $(CFLAGS) -o $@ $^

sprintf.elf: sprintf.o $(LIBM)
	$(CC) $(CFLAGS) -o $@ $^

scanf.elf: scanf.o $(LIB)
	$(CC) $(CFLAGS) -o $@ $^

tmpfile.elf: tmpfile.o $(LIB)
	$(CC) $(CFLAGS) -o $@ $^


mutex.elf: mutex.c $(LIBPTHREAD)
	$(CC) $(CFLAGS) -o $@ $^

%.run: %.elf
	propeller-load -bc3 $^ -r -t -q

clean:
	rm -rf *.o *.elf
