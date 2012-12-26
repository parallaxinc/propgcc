all:	fibotest.elf fibotest.bin

CC=propeller-elf-gcc

#FILES=fibotest.o
FILES=fibo.o

CFLAGS=-Os -mp2 -mno-fcache

fibotest.elf:	$(FILES)
	$(CC) $(CFLAGS) -o $@ $(FILES)

%.o:	%.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o:	%.S
	$(CC) $(CFLAGS) -c $< -o $@

fibotest.bin:	fibotest.elf
	propeller-elf-objcopy -O binary fibotest.elf fibotest.bin

run:
	p2load -v -s -h fibotest.bin -t

clean:
	rm -f *.o fibotest.elf fibotest.bin
