propeller-elf-gcc -Wall -Os -m32bit-doubles -o calc.elf calc.c -lm
propeller-load -r -t calc.elf
