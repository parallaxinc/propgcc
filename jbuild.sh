#!/bin/sh
#
# run this script in this directory (the top level propgcc)
#

#
# ADJUST THE FOLLOWING VARIABLES IF NECESSARY
#
echo Prefix is ${PREFIX=/opt/parallax}
export PREFIX

#
# note that the propgcc version string does not deal well with
# spaces due to how it is used below
#
echo Propgcc version is ${PROPGCC_VERSION='propgcc_v0.3.1'}
export PROPGCC_VERSION

echo BUGURL is ${BUGURL="http://code.google.com/p/propgcc/issues"}
export BUGURL

#
# configure options for propgcc
#
CONFIG_OPTIONS="--with-pkgversion=${PROPGCC_VERSION} --with-bugurl=${BUGURL}"

#
# ADD build target bin directory to PATH
#
PATH=$PREFIX/bin:$PATH
export PATH

#
# we want to specify the number of jobs to use for multi-core cpus
#
if test ARG$1 = ARG
then
   echo "Usage: <number of jobs> [rm]"
   echo "Building without rm or multiple jobs."
#
# if only one job requested, don't use J
#
elif test ARG$1 = ARG1
then
   JOBS=""
else
   JOBS="-j ${1}"
   echo "Building with: ${JOBS}"
fi

#
# if we have an rm in $2, build from scratch
#
if test ARG$2 = ARGrm
then
   echo "Removing old build."
   rm -rf ../build
fi

export JOBS

#
# attempt to auto-detect the OS
UNAME=`uname -s`
if test NAME`echo $UNAME | grep "[_-]"` != NAME
then
  # if system has [-_] try uname -o ... Cygwin for example
  UNAME=`uname -o`
fi
echo "OS '$UNAME' detected."

if test x$UNAME = xDarwin
then
  OS=macosx
  PORT=/dev/cu.usbserial-A8004ILf
  BOARD=hub
  BSTC=bstc.osx
elif test x$UNAME = xCygwin
then
  OS=cygwin
  PORT=COM16
  BOARD=c3
  BSTC=bstc.exe
elif test x$UNAME = xMsys
then
  OS=msys
  PORT=COM16
  BOARD=c3
  BSTC=bstc.exe
elif test x$UNAME = xLinux
then
  OS=linux
  PORT=/dev/ttyUSB0
  BOARD=c3
  BSTC=bstc.linux
else
  echo "Unknown system: " $UNAME
  exit 1
fi

#
# We have a valid system. export variables.
# Bourne shell must set and export separately for some linux.
# This seems to be an issue with debian ....
#
export OS
export PORT
export BOARD

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

# do this incase we texi needs it
mkdir -p ../build/binutils/etc
cp gnu-oids.texi ../build/binutils/etc

cd ../build/binutils
../../propgcc/binutils/configure --target=propeller-elf --prefix=$PREFIX --disable-nls ${CONFIG_OPTIONS}
if test $? != 0
then
   echo "binutils configure failed."
   cd ../../propgcc
   exit 1
fi
make ${JOBS} all
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
../../propgcc/gcc/configure --target=propeller-elf --prefix=$PREFIX --disable-nls --disable-libssp --disable-lto ${CONFIG_OPTIONS}
if test $? != 0
then
   echo "gcc configure failed."
   cd ../../propgcc
   exit 1
fi
make ${JOBS} all-gcc
if test $? != 0
then
   echo "gcc make all-gcc failed."
   cd ../../propgcc
   exit 1
fi
make install-gcc
if test $? != 0
then
   echo "gcc make install-gcc failed."
   cd ../../propgcc
   exit 1
fi
cd ../../propgcc

#
# build newlibs
#
#mkdir -p ../build/newlib
#cd ../build/newlib
#../../propgcc/newlib/src/configure --target=propeller-elf --prefix=$PREFIX --enable-target-optspace
#if test $? != 0
#then
#   echo "newlib configure failed."
#   cd ../../propgcc
#   exit 1
#fi
#make all
#if test $? != 0
#then
#   echo "newlib make all failed."
#   cd ../../propgcc
#   exit 1
#fi
#make install
#if test $? != 0
#then
#   echo "newlib make install failed."
#   cd ../../propgcc
#   exit 1
#fi

#
# build library
#
cd lib
make clean
if test $? != 0
then
  echo "library build failed - make clean"
  cd ..
  exit 1
fi
make PREFIX=$PREFIX ${JOBS}
if test $? != 0
then
  echo "library build failed - make"
  cd ..
  exit 1
fi
make PREFIX=$PREFIX install
if test $? != 0
then
  echo "library install failed"
  cd ..
  exit 1
fi
cd ..

#
# copy the linker scripts
#
cp -f ldscripts/* $PREFIX/propeller-elf/lib
if test $? != 0
then
  echo "ldscripts install failed"
  exit 1
fi

#
# copy Brad's Spin Tool
#
cp -f release/$BSTC $PREFIX/bin
if test $? != 0
then
  echo "bstc install failed"
  exit 1
fi
chmod a+x $PREFIX/bin/$BSTC
if test $? != 0
then
  echo "bstc chmod failed"
  exit 1
fi

#
# build gcc libgcc
# this must be done after the library build, since it depends on
# library header files
#
cd ../build/gcc
make ${JOBS} all-target-libgcc
if test $? != 0
then
   echo "gcc make all failed"
   cd ../../propgcc
   exit 1
fi
make install-target-libgcc
if test $? != 0
then
   echo "gcc make install failed."
   cd ../../propgcc
   exit 1
fi
cd ../../propgcc

#
# build gcc libstdc++
# this must be done after the library build, since it depends on
# library header files
#
cd ../build/gcc
make ${JOBS} all
if test $? != 0
then
   echo "gcc make all failed"
   cd ../../propgcc
   exit 1
fi
make install
if test $? != 0
then
   echo "gcc make install failed."
   cd ../../propgcc
   exit 1
fi
cd ../../propgcc

#
# build propeller-load ... before gdb
# gdbstub relies on a loader library
#
make -C loader clean
if test $? != 0
then
   echo "loader make clean failed"
   exit 1
fi

make -C loader TARGET=$PREFIX BUILDROOT=../../build/loader
if test $? != 0
then
   echo "loader make failed"
   exit 1
fi

make -C loader TARGET=$PREFIX BUILDROOT=../../build/loader install
if test $? != 0
then
   echo "loader install failed"
   exit 1
fi

#
# build propeller-elf-gdb
#
mkdir -p ../build/gdb
cd ../build/gdb
../../propgcc/gdb/configure --target=propeller-elf
if test $? != 0
then
   echo "gdb configure failed"
   cd ../../propgcc
   exit 1
fi

make ${JOBS} all
if test $? != 0
then
   echo "gdb make all failed"
   cd ../../propgcc
   exit 1
fi
if [ ${OS} != "msys" ]
then
    cp -f gdb/gdb ${PREFIX}/bin/propeller-elf-gdb
else
    cp -f gdb/gdb.exe ${PREFIX}/bin/propeller-elf-gdb.exe
fi
cd ../../propgcc

#
# build spinsim
#
cp -r spinsim ../build/.
cd ../build/spinsim
make clean
if test $? != 0
then
   echo "spinsim clean failed"
   cd ../../propgcc
   exit 1
fi
make
if test $? != 0
then
   echo "spinsim make failed"
   cd ../../propgcc
   exit 1
fi
if [ ${OS} != "msys" ]
then
    cp -f spinsim ${PREFIX}/bin/.
else
    cp -f spinsim.exe ${PREFIX}/bin/.
fi
cd ../../propgcc

#
# build gdbstub
#
cd gdbstub
make clean
if test $? != 0
then
   echo "gdbstub clean failed"
   cd ..
   exit 1
fi
make
if test $? != 0
then
   echo "gdbstub make failed"
   cd ..
   exit 1
fi
if [ ${OS} != "msys" ]
then
    cp -f gdbstub ${PREFIX}/bin/.
else
    cp -f gdbstub.exe ${PREFIX}/bin/.
fi
cd ../../propgcc

echo "Build complete."
exit 0
