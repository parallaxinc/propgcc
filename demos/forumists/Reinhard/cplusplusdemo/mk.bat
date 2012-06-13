propeller-elf-gcc -mlmm -o SquareWaveGenerator.o  -c SquareWaveGenerator.cpp

propeller-elf-gcc -mlmm -o cplusplusdemo.elf  cplusplusdemo.cpp SquareWaveGenerator.o

propeller-load -r cplusplusdemo.elf
