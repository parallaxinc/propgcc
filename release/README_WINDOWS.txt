I. To install:

  Copy the propgcc directory to the root directory of the C drive.

II. To use Propeller-GCC from the windows command window use one of these methods:

  A. From the new c:\propgcc folder in windows exporer - recommended:

    1. Double click C:\propgcc\PropGCC.bat to start a new window.
    2. Use commands mentioned below.

  -- OR --

  B. Use PropGCC.bat

    1. C:\>cd propgcc
    2. C:\propgcc>
    3. C:\propgcc>PropGCC
    4. Use commands mentioned below in the new window.

  -- OR --

  C. Use addpath.bat

    1. C:\>cd propgcc
    2. C:\propgcc>
    3. C:\propgcc>addpath
    4. Use commands mentioned below in the existing window.

    This would be done every time a new command window is started.
    Option A or B above automates these steps.


III. Using Propeller-GCC

There are some demo programs located in the demos directory.  You can build
all the demo programs by going into the demos directory and typing make.  If
you want to build a specific program you can run make from the directory for
that program.

As an example, the fibo/lmm directory contains the Fibonnaci series program
that executes LMM code from hub memory.  You can go into this directory and
run make to build it.  After the program is built it can be downloaded to a
Propeller board and executed as follows:

  propeller-load -p com# fibo.elf -r -t

  propeller-load is the propgcc loader program.
  -p com# is the name of the serial port, such as com7.
     if you omit -p the loader will use the first serial port in the system.
  fibo.elf is the program image file to load.
  -r says load to ram and run the program.
  -e would mean load to eeprom and run the program.
  -t says start the terminal (press ESC or Ctrl-C when done).

  You can omit -p com# and propeller-load will use the first port for download.
  propeller-load fibo.elf -r -t

The fibo program also has XMM and XMMC versions.  These programs will run on
boards that contain external memory.  A board type must be specified when
loading these programs as follows:

  propeller-load -b board fibo.elf -r -t

  If you don't use -t, you will not get the terminal.
  Press ESC to exit the terminal.


Single .c file programs can be built and run like this:

Create a file hello.c:

#include <stdio.h>
int main()
{
    printf("Hello World.\n");
    return 0;
}

C:\propgcc>propeller-elf-gcc -o hello hello.c

One can use propeller-load to load the program and start the terminal.
For example use: propeller-load hello -r -t

C:\propgcc>propeller-load hello -r -t
Propeller Version 1 on com24
Writing 13536 bytes to Propeller RAM.
Verifying ... Upload OK!
[ Entering terminal mode. Type ESC or Control-C to exit. ]
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

C:\propgcc>propeller-elf-gcc -o blink blink.c
C:\propgcc>propeller-load blink -r

Some currently supported board types are:

hub       - 80 MHz board without external memory
c3        - 80 MHz C3 card
ssf       - 80 MHz SpinSocket Flash with 4MB Flash
dracblade - 80 MHz DracBlade SBC with SRAM
sdram     - 80 MHz GadgetGangster 32MB SDRAM module


Any board type clock frequency can be changed using a -Dclkfreq flag.

  propeller-load fibo.elf -r -t -Dclkfreq=96000000
  propeller-load fibo.elf -r -t -Dclkfreq=100000000


Any board type clock mode can be changed using a -Dclkmode flag.

  propeller-load fibo.elf -r -t -Dclkmode=XTAL1+PLL8X
  propeller-load fibo.elf -r -t -Dclkfreq=96000000 -Dclkmode=XTAL1+PLL8X


