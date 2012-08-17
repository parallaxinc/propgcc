NAME = calc

ifndef MODEL
MODEL = lmm
endif

$(NAME).elf: Makefile $(NAME).c
	propeller-elf-gcc -m$(MODEL) -Wall -Os -m32bit-doubles -o $(NAME).elf $(NAME).c -lm

clean:
	rm -f *.o *.elf

