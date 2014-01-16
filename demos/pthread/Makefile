ifndef MODEL
MODEL = lmm
endif

CC = propeller-elf-gcc
CFLAGS = -Os -m$(MODEL)

all: thread.elf pingpong.elf

thread.elf: thread.c
	$(CC) -o $@ $(CFLAGS) $^ -lpthread

pingpong.elf: pingpong.c
	$(CC) -o $@ $(CFLAGS) $^ -lpthread

clean:
	rm -rf *.elf *.o
