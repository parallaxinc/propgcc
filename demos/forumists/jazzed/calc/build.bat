propeller-elf-gcc -Wall -Os -m32bit-doubles calc.c -lm -s
propeller-load -r -t a.out
