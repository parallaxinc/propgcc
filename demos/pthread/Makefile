ifndef MODEL
MODEL = lmm
endif

CC = propeller-elf-gcc
CFLAGS = -Os -m$(MODEL)

thread.elf: thread.c
	$(CC) -o $@ $(CFLAGS) $^ -lpthread

clean:
	rm -rf *.elf *.o
