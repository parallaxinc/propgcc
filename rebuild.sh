#!/bin/sh
#
# run this script in this directory (the top level propgcc)
#

#
# if we have an argument don't remove build
#
if test NOARG$1 = NOARG
then
   echo "Removing old build."
   rm -rf ../build
fi

#
# build binutils
#
mkdir -p ../build/binutils
cd ../build/binutils
../../propgcc/binutils/configure --target=propeller-elf --prefix=/usr/local/propeller --disable-nls
if test $? != 0
then
   echo "binutils configure failed."
   cd ../../propgcc
   exit 1
fi
make all
if test $? != 0
then
   echo "binutils make failed."
   cd ../../propgcc
   exit 1
fi
make install
if test $? != 0
then
   echo "binutils make install failed."
   cd ../../propgcc
   exit 1
fi
cd ../../propgcc

#
# build gcc
#
mkdir -p ../build/gcc
cd ../build/gcc
../../propgcc/gcc/configure --target=propeller-elf --prefix=/usr/local/propeller --disable-nls --disable-libssp
if test $? != 0
then
   echo "gcc configure failed."
   cd ../../propgcc
   exit 1
fi
make all-gcc
if test $? != 0
then
   echo "gcc make all-gcc failed."
   cd ../../propgcc
   exit 1
fi
make install-gcc
if test $? != 0
then
   echo "binutils make install-gcc failed."
   cd ../../propgcc
   exit 1
fi

make all-target-libgcc
if test $? != 0
then
   echo "libgcc make all-target-libgcc failed."
   cd ../../propgcc
   exit 1
fi
make install-target-libgcc
if test $? != 0
then
   echo "libgcc make install-target-libgcc failed."
   cd ../../propgcc
   exit 1
fi
cd ../../propgcc

#
# build newlibs
#
mkdir -p ../build/newlib
cd ../build/newlib
../../propgcc/newlib/src/configure --target=propeller-elf --prefix=/usr/local/propeller --enable-target-optspace
if test $? != 0
then
   echo "newlib configure failed."
   cd ../../propgcc
   exit 1
fi
make all
if test $? != 0
then
   echo "newlib make all failed."
   cd ../../propgcc
   exit 1
fi
make install
if test $? != 0
then
   echo "newlib make install failed."
   cd ../../propgcc
   exit 1
fi

echo "Build complete."
exit 0
