This is a project for the port of GCC (the GNU Compiler Collection) to
the Parallax Propeller.

Please use the Makefile to build Propeller GCC.

To build from scratch use:

    $ make

All the files will be placed in a ../build directory. You can also
build individual components, e.g "make gdb" to build gdb.

Cross compilation from Linux to Windows or Raspberry Pi is also
supported by the Makefile, e.g. "make CROSS=win32" or "make
CROSS=rpi". See README.cross for more details.

Output is placed in /opt/parallax by default (/opt/parallax-win32 or
/opt/parallax-rpi for cross compiles). You can change this
by saying "make PREFIX=<output directory>".

------------------------------------------------------------------
Directory organization is as follows:

    binutils  - binutils 2.23.1 (complete) plus propeller changes
    demos     - some demos for the compiler (see below)
    doc       - some propeller specific library documentation
    gcc	  - gcc 4.6.1 (complete) plus propeller changes
    gdb       - gdb 7.3.1 plus propeller changes
    gdbstub   - interface between gdb and the propeller
    lib       - propeller libraries
    loader    - propeller-load serial loader and communications program
    ncurses   - terminal library; needed to build gdb on Raspberry Pi
    newlib    - a different C library (not currently used)
    spin2cpp  - a tool to convert Spin programs to C or C++
    spinsim   - Dave Hein's Propeller simulator

--------------------------- demos ---------------------------------

A number of demos are provided. In each directory there should be a
Makefile. To build the demo do "make"; to run it do "make run".

Of particular interest is the 
demo/toggle directory, containing a number of different ways of
toggling an LED, using one or multiple COGs.
 
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
