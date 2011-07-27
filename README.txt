This is a project for the port of GCC (the GNU Compiler Collection) to
the Parallax Propeller.

Directory organization is as follows:

binutils  - binutils 2.21 (complete) plus propeller changes
gcc	  - gcc 4.6.1 (complete) plus propeller changes


----------------------- binutils build -------------------------------
To build under Linux, execute the following commands (starting in this
directory, which I assume is named "propgcc" on your system):

(1) mkdir -p ../build/binutils
(2) cd ../build/binutils
(3) ../../propgcc/binutils/configure --target=propeller-elf
(4) make

This will start building binutils. It will fail when it gets to things
we haven't gotten finished yet (the disassembler, linker, and/or gas).


------------------------ gcc build ---------------------------------
To build under Linux, execute the following commands (starting in this
directory, which I assume is named "propgcc" on your system):

(1) mkdir -p ../build/gcc
(2) cd ../build/gcc
(3) ../../propgcc/gcc/configure --target=propeller
(4) make

This will fail with a message about object file types when it gets to
the point of trying to build the gcc front end driver. But the compiler
itself (cc1) is built at this point, and we can use it like so:

gcc/cc1 -O -fomit-frame-pointer -o foo.s foo.c

to produce assembly output from (already preprocessed) C code.

