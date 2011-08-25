This is a project for the port of GCC (the GNU Compiler Collection) to
the Parallax Propeller.

Directory organization is as follows:

binutils  - binutils 2.21 (complete) plus propeller changes
gcc	  - gcc 4.6.1 (complete) plus propeller changes


----------------------- binutils build -------------------------------
To build under Linux, execute the following commands (starting in this
directory, which I assume is named "propgcc" on your system):

(1) mkdir -p ../build
(2) cd ../build
(3) ../propgcc/binutils/configure --target=propeller-elf --prefix=/usr/local/propeller --disable-nls
(4) make all

This will build binutils. 

To install it in /usr/local/propeller (make sure that directory exists
on your system and is writable first!) do:

(5) make install

------------------------ gcc build ---------------------------------
To build under Linux, execute the following commands (starting in this
directory, which I assume is named "propgcc" on your system):

(1) mkdir -p ../build/gcc
(2) cd ../build/gcc
(3) ../../propgcc/gcc/configure --target=propeller-elf --prefix=/usr/local/propeller --disable-nls
(4) make all-gcc
(5) make install-gcc

