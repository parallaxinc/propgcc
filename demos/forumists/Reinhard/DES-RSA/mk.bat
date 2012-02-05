propeller-elf-gcc   -mlmm -o crypto_demo.elf   ex-1.c bit.c des.c rsa.c

propeller-load -r -t crypto_demo.elf
