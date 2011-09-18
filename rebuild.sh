#!/bin/sh
#
# run this script in this directory (the top level propgcc)
#
rm -rf ../build

#
# build binutils
#
mkdir -p ../build/binutils
cd ../build/binutils
../../propgcc/binutils/configure --target=propeller-elf --prefix=/usr/local/propeller --disable-nls
make all
make install
cd ../../propgcc

#
# build gcc
#
mkdir -p ../build/gcc
cd ../build/gcc
../../propgcc/gcc/configure --target=propeller-elf --prefix=/usr/local/propeller --disable-nls --disable-libssp
make all-gcc
make install-gcc

make all-target-libgcc
make install-target-libgcc
cd ../../propgcc

#
# build newlibs
#
mkdir -p ../build/newlib
cd ../build/newlib
../../propgcc/newlib/src/configure --target=propeller-elf --prefix=/usr/local/propeller
make all
make install
