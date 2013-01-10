all:	fibo.elf

CC=propeller-elf-gcc

FILES=fibo.o

MODEL?=lmm

CFLAGS=-Os -m$(MODEL) -mp2 -mno-fcache

fibo.elf:	$(FILES)
	$(CC) $(CFLAGS) -o $@ $(FILES)

%.o:	%.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o:	%.S
	$(CC) $(CFLAGS) -c $< -o $@

run:
	p2load -v fibo.elf -t

clean:
	rm -f *.o fibo.elf
