# Introduction #

> Two types of demo program sets will be available:
    1. Makefile Demos
    1. Eclipse Demos

> Makefile demos use simple make scripts. Eclipse demos will use the built-in Eclipse project manager.

> Instructions for using other IDEs project managers may be added in due time.

> If you are using the propgcc repository for builds instead of a release package, you should copy the appropriate system bstc file from propgcc/release to a directory in your path. Make a copy of bstc.linux or bstc.osx there to "bstc" with no extension.


## Using Makefile Demos ##

> People fear, loathe, and love makefiles. Propeller GCC demos currently use makefiles because the developers don't mind using them. The demo makefiles are very simple.

> Using makefiles is much easier than writing makefiles. The author is not trying to convert anyone, it's just that there are some simple demos that use make.

> Assuming you have downloaded a package and followed installation instructions, you can build the demos as follows below. The make process requires a command console window (or an IDE that runs make in the background).

> To build all demos in the default mode for the demo, in a console window, go to the demo folder used during install.
    1. $ cd demos
    1. $ make

> To make all demos from scratch:
    1. $ make clean
    1. $ make

> Alternatively to make all demos from scratch:
    1. $ make clean; make

> To build the fft\_bench demo for LMM
    1. $ cd demos/fft
    1. $ make clean
    1. $ make

> To build the fft\_bench demo for XMMC
    1. $ cd demos/fft
    1. $ make clean
    1. $ make MODEL=XMMC

> Some demos have separate directories for the different MODEL builds.
    1. $ cd demos/fibo/cog
    1. $ make
    1. $ cd demos/fibo/lmm
    1. $ make
    1. $ cd demos/fibo/xmmc
    1. $ make
    1. $ cd demos/fibo/xmm
    1. $ make

> Many makefiles allow you to build the program and download it to your Propeller. To do it all in one command, PORT and BOARD must be specified. You can also use [propeller-load](PropGccLoader.md) separately.

> The PORT for linux can be a simple number (say #) which will expand by default to /dev/ttyUSB#. I.E. PORT=0 says use /dev/ttyUSB0. For MAC users, it's best to set PORT to the name given by OSX to your port.

> The BOARD should be set to HUB for LMM programs or one of the appropriate [propeller-load.cfg](PropGccLoader.md) boards for XMM programs. I.E. BOARD=hub (for LMM programs) or BOARD=eeprom (for XMMC programs).
```
Begin Example

$ make PORT=0 BOARD=hub run
rm -f *.o *.elf *.cog *.binary
propeller-elf-gcc -O2 -mfcache -mlmm -o fft_bench.o -c fft_bench.c
propeller-elf-gcc -mlmm  -o fft_bench.elf fft_bench.o
propeller-load -p0 -bhub fft_bench.elf -r -t
Propeller Version 1 on /dev/ttyUSB0
Writing 25580 bytes to Propeller RAM.
Verifying ... Upload OK!
fft_bench v1.0
Freq.    Magnitude
00000000 1fe
000000c0 1ff
00000140 1ff
00000200 1ff
1024 point bit-reversal and butterfly run time = 47 ms

End Example
```


## Using Eclipse Demos ##

> Work in progress ...