@file propgcc/demos/README.txt

This directory contains early demos and other programs contributed by users.

Early Demos:
./common                : contains common make files
./dhrystone             : dhrystone 2.2 test
./fft                   : heater's fft_bench program
./fibo                  : top level fibo test program directory
./fibo/cog              : GCC cog only fibo
./fibo/lmm              : LMM fibo test
./fibo/xmm              : XMM fibo test
./fibo/xmmc             : XMMC fibo test
./forumists/*           : Parallax forum user contributions.
./include               : include files for libpropeller
./libpropeller          : the small propeller library
./toggle                : top level toggle program directory
./toggle/cog_c_toggle   : currently broken - will fix later
./toggle/c++_toggle     : c++ version of toggle
./toggle/gas_toggle     : gas version of toggle
./toggle/lmm_c_toggle   : lmm lanching a second lmm version of toggle
./toggle/lmm_toggle     : lmm version of toggle
./toggle/pasm_toggle    : pasm version of toggle
./toggle/xmm_toggle     : xmm version of toggle
./TV                    : top level TV program directory
./TV/TvText_demo        : display TV with DAC on pins 12-15
./xbasic                : embedded version of the xbasic language

Each program can be compiled with one of 3 current default linker models:
lmm, xmm, and xmmc. The cog programs have slightly different requirements.
Users can define their own linker scripts. The xmm and xmmc models require
either a 64KB+ EEPROM or special external memory hardware.

  In demos that have only a top level directory, the model build is invoked as:
make (default MODEL=lmm), make MODEL=lmm, make MODEL=xmm, or make MODEL=xmmc.
make clean removes .o and .elf files.

An Eclipse workspace directory is planned where makefiles will not be necessary.

Any single file program can be compiled and linked with propeller-elf-gcc as:
propeller-elf-gcc -o output source.c

The propeller-load program is used to download elf images to the propeller hardware.
Use propeller-load -h to get a brief summary of options.

usage: propeller-elf-load
         [ -b <type> ]     select target board (default is default)
         [ -p <port> ]     serial port (default is /dev/ttyUSB0)
         [ -I <path> ]     add a directory to the include path
         [ -D var=value ]  define a board configuration variable
         [ -e ]            write the program into EEPROM
         [ -r ]            run the program after loading
         [ -s ]            write a spin binary file for use with the Propeller Tool
         [ -t ]            enter terminal mode after running the program
         [ -t<baud> ]      enter terminal mode with a different baud rate
         <name>            file to compile

Variables that can be set with -D are:
  clkfreq clkmode baudrate rxpin txpin tvpin cache-driver cache-size cache-param1 cache-param2

Some simple examples of propeller-load:

The first four load LMM dry.elf to 80MHz Propeller HUB

propeller-load -r -t dry.elf                    : use first port
propeller-load -r -t dry.elf -p 0               : use COM0 or /dev/ttyUSB0
propeller-load -r -t dry.elf -p 1               : use COM1 or /dev/ttyUSB1
propeller-load -r -t dry.elf -p /dev/ttyUSB1    : use /dev/ttyUSB0

These load LMM dry.elf to 96MHz Propeller HUB

propeller-load -r -t dry.elf -p 0 -Dclkfreq=96000000
propeller-load -r -t dry.elf -p 0 -Dclkfreq=96000000 -b hub

These load XMM or XMMC compiled dry.elf Propeller non-volatile EEPROM or Flash

propeller-load -r -t dry.elf -p 0 -Dclkfreq=80000000 -b eeprom
propeller-load -r -t dry.elf -p 0 -Dclkfreq=80000000 -b c3
propeller-load -r -t dry.elf -p 0 -Dclkfreq=96000000 -b ssf

This loads XMMC compiled dry.elf Propeller to SDRAM

propeller-load -r -t dry.elf -p 0 -b sdram

