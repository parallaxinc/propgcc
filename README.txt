This is a project for the port of GCC (the GNU Compiler Collection) to
the Parallax Propeller.

Directory organization is as follows:

binutils  - binutils 2.21 (complete) plus propeller changes
gcc	  - gcc 4.6.1 (complete) plus propeller changes
demos     - some demos for the compiler (see below)

To build, build binutils first, then gcc, then demos.

----------------------- binutils build -------------------------------
To build under Linux, execute the following commands (starting in this
directory, which I assume is named "propgcc" on your system):

(1) mkdir -p ../build/binutils
(2) cd ../build/binutils
(3) ../../propgcc/binutils/configure --target=propeller-elf --prefix=/usr/local/propeller --disable-nls 
(4) make all

This will build binutils. 

To install it in /usr/local/propeller (make sure that directory exists
on your system and is writable first!) do:

(5) make install

I also suggest building and installing "propeller-checksum", which is
a program to set the checksum on binaries; without this the demos can
only be run on a simulator, not real hardware.

(6) gcc -o /usr/local/propeller/bin/propeller-checksum propeller-checksum.c

------------------------ gcc build ---------------------------------
To build under Linux, execute the following commands (starting in this
directory, which I assume is named "propgcc" on your system):

(1) mkdir -p ../build/gcc
(2) cd ../build/gcc
(3) ../../propgcc/gcc/configure --target=propeller-elf --prefix=/usr/local/propeller --disable-nls --disable-libssp
(4) make all-gcc
(5) make install-gcc

Now we can build libgcc

(6) make all-target-libgcc
(7) make install-target-libgcc

--------------------------- demos ---------------------------------

The following demos are provided. In each directory there should be a
Makefile. To build the demo do "make"; to run it do "make run".

fibo  - Generate Fibonacci numbers and print them, along with timings
        This is available both in a cog only and hub (LMM) version;
        see the appropriate subdirectories of "fibo". The Makefiles
        are set up to load and run with Dave Hein's SPINSIM simulator,
        but the binaries should also work on a C3 or similar platform.


 
