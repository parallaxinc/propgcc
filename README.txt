This is a project for the port of GCC (the GNU Compiler Collection) to
the Parallax Propeller.

Please use the rebuild.sh script to build Propeller GCC.

    To build from scratch use:
    $ ./rebuild.sh

    To build without removing the build directory use:
    $ ./rebuild.sh 1

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
(3) ../../propgcc/binutils/configure --target=propeller-elf --prefix=/opt/parallax --disable-nls 
(4) make all

This will build binutils. 

To install it in /opt/parallax (make sure that directory exists
on your system and is writable first!) do:

(5) make install

Please note that propeller-checksum is no longer used. Propeller-load is used instead.

(6) cd propgcc/loader; source setenv.linux; make clean; make; make install

------------------------ gcc build ---------------------------------
To build under Linux, execute the following commands (starting in this
directory, which I assume is named "propgcc" on your system):

(1) mkdir -p ../build/gcc
(2) cd ../build/gcc
(3) ../../propgcc/gcc/configure --target=propeller-elf --prefix=/opt/parallax --disable-nls --disable-libssp
(4) make all-gcc
(5) make install-gcc

Now we can build libgcc

(6) make all-target-libgcc
(7) make install-target-libgcc

-------------------------- newlib ---------------------------------

Newlib is no longer active. Ignore this step.
It is kept here only for future reference if required.

To build under Linux, execute the following commands (starting in this
directory, which I assume is named "propgcc" on your system):

(1) mkdir -p ../build/newlib
(2) cd ../build/newlib
(3) ../../propgcc/newlib/src/configure --target=propeller-elf --prefix=/opt/parallax --enable-target-optspace
(4) make all

This will build newlib. 

To install it in /opt/parallax (make sure that directory exists
on your system and is writable first!) do:

(5) make install

-------------------------- Propeller GCC library ------------------

To build under Linux, execute the following commands (starting in this
directory, which I assume is named "propgcc" on your system):

(1) cd lib
(2) make clean
(3) make
(4) make install

--------------------------- demos ---------------------------------

The following demos are provided. In each directory there should be a
Makefile. To build the demo do "make"; to run it do "make run".

fibo  - Generate Fibonacci numbers and print them, along with timings
        This is available both in a cog only and hub (LMM) version;
        see the appropriate subdirectories of "fibo". The Makefiles
        are set up to load and run with Dave Hein's SPINSIM simulator,
        but the binaries should also work on a C3 or similar platform.


 
--------------------------- test suites ---------------------------------

Copy the contents of propgcc/dejagnu to
/opt/parallax/share/dejagnu.

Set the environment variable DEJAGNU to
/opt/parallax/share/dejagnu/site.exp.

If you are not using a C3 board, edit
/opt/parallax/share/dejagnu/board/propeller-sim.exp to reflect the
actual board you are using. The propeller-load options are given in
the line that reads:
   set_board_info sim,options "-bc3 -r -t -q"
Modify the command line to whatever you need to run XMM programs with
propeller-load on your system.

Make sure that your Propeller board is connected and turned on, then
in build/gcc, do:
   make check-gcc RUNTESTFLAGS="--target_board=propeller-sim"

--------------------------- .pdf documentation --------------------------

in build/gcc do:
  make pdf
  make install-pdf

This will copy various .pdf files to the /opt/parallax/share/doc
directory. The ones in the root of that directory (gmp.pdf, libiberty.pdf,
libquadmath.pdf, mpc.pdf, and mpfr.pdf) are not really interesting,
since they are for libraries used in building the compiler.

The ones in the gcc/ subdirectory, in particular gcc/cpp.pdf and
gcc/gcc.pdf, are the "real" compiler documents. They do have some
propeller specific information in them, although more is needed.

