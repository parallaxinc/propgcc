ifndef MODEL
MODEL = lmm
endif

CC = propeller-elf-gcc
CFLAGS = -Os -m$(MODEL)

thread.elf: thread.c
	$(CC) -o $@ $(CFLAGS) $^

clean:
	rm -rf *.elf *.o
