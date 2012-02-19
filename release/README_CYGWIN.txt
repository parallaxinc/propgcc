To install and use this package for use with Cygwin, please move or copy the
propeller directory to /opt/parallax.

$ mv propeller /opt/parallax/.
$ PATH=/opt/parallax/bin:$PATH

Add the PATH setting to your .bashrc file for future use.

There are a few demo programs located in the demos directory.  You can build
all the demo programs by going into the demos directory and typing make.  If
you want to build a specific program you can run make from the directory for
that program.

As an example, the fibo/lmm directory contains the Fibonnaci series program
that executes LMM code from hub memory.  You can go into this directory and
run make to build it.  After the program is built it can be downloaded to a
Propeller board and executed as follows:

  $ propeller-load -p com# fibo.elf -r -t

  propeller-load is the propgcc loader program.
  -p com# is the name of the serial port, such as com7.
  fibo.elf is the program image file to load.
  -r says load to ram and run the program.
  -e would mean load to eeprom and run the program.
  -t says start the terminal (press ESC or Ctrl-C when done).

The fibo program also has XMM and XMMC versions.  These programs will run on
boards that contain external memory.  A board type must be specified when
loading these programs as follows:

  $ propeller-load -p com# -b board fibo.elf -r -t

  If you don't use -t, you will not get the terminal.
  Press ESC to exit the terminal.


Single .c file programs can be built and run like this:

Create a file hello.c:

#include <stdio.h>
int main()
{
    printf("Hello World.");
    return 0;
}

$ propeller-elf-gcc -o hello hello.c

One can use propeller-load to load the program and start the terminal.
For example use: propeller-load -p com25 hello -r -t
Of course, replace com25 with your port number.

$ propeller-load -p com24 hello -r -t
Propeller Version 1 on com24
Writing 13536 bytes to Propeller RAM.
Verifying ... Upload OK!
Hello World.
------- Press ESC -------
C:\propgcc>


Create a file blink.c:
#include <propeller.h>

int main()
{
    int n = 0;
    DIRA  = 0xFFFF;
    while(1) {
        OUTA ^= DIRA;
        waitcnt(CLKFREQ+CNT);
    }
    return 0;
}

$ propeller-elf-gcc -o blink blink.c
$ propeller-load -p com25 blink -r

  Replace com25 with your port number.


Some currently supported board types are:

hub       - 80 MHz board without external memory
c3        - 80 MHz C3 card
ssf       - 80 MHz SpinSocket Flash with 4MB Flash
dracblade - 80 MHz DracBlade SBC with SRAM
sdram     - 80 MHz GadgetGangster 32MB SDRAM module


Any board type clock frequency can be changed using a -Dclkfreq flag.

  $ propeller-load -p com# -b board fibo.elf -r -t -Dclkfreq=96000000
  $ propeller-load -p com# -b board fibo.elf -r -t -Dclkfreq=100000000


Any board type clock mode can be changed using a -Dclkmode flag.

  $ propeller-load -p com# -b board fibo.elf -r -t -Dclkmode=XTAL1+PLL8X
  $ propeller-load -p com# -b board fibo.elf -r -t -Dclkfreq=96000000 -Dclkmode=XTAL1+PLL8X


