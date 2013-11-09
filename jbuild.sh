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
GCCSRC=`basename $(pwd)`

#
# note that the propgcc version string does not deal well with
# spaces due to how it is used below
#
VERSION=`cat release/VERSION.txt | grep -v "^#"`
# better revision command. thanks yeti.
HGVERSION=`hg tip --template '{rev}\n'`

PROPGCC_VERSION=`echo ${VERSION}_${HGVERSION}`

echo Propgcc version is ${PROPGCC_VERSION}
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
   echo "Usage: <number of jobs> [rm] or [rm-all]"
   echo "Building without rm, rm-all, or multiple jobs."
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

#
# if we have an rm-all in $2, build from scratch
#
if test ARG$2 = ARGrm-all
then
   echo "Removing old build and $PREFIX/*."
   rm -rf ../build
   rm -rf $PREFIX/*
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
  SPINCMP=openspin.osx
elif test x$UNAME = xCygwin
then
  OS=cygwin
  PORT=COM16
  BOARD=c3
  SPINCMP=openspin.exe
elif test x$UNAME = xMsys
then
  OS=msys
  PORT=COM16
  BOARD=c3
  SPINCMP=openspin.exe
elif test x$UNAME = xLinux
then
  OS=linux
  PORT=/dev/ttyUSB0
  BOARD=c3
  SPINCMP=openspin.linux
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
../../$GCCSRC/binutils/configure --target=propeller-elf --prefix=$PREFIX --disable-nls --disable-shared ${CONFIG_OPTIONS}
if test $? != 0
then
   echo "binutils configure failed."
   cd ../../$GCCSRC
   exit 1
fi
make ${JOBS} all
if test $? != 0
then
   echo "binutils make failed."
   cd ../../$GCCSRC
   exit 1
fi
make install
if test $? != 0
then
   echo "binutils make install failed."
   cd ../../$GCCSRC
   exit 1
fi
cd ../../$GCCSRC

#
# build gcc
#
mkdir -p ../build/gcc
cd ../build/gcc
../../$GCCSRC/gcc/configure --target=propeller-elf --prefix=$PREFIX --disable-nls --disable-libssp --disable-lto --disable-shared ${CONFIG_OPTIONS}
if test $? != 0
then
   echo "gcc configure failed."
   cd ../../$GCCSRC
   exit 1
fi
make ${JOBS} all-gcc
if test $? != 0
then
   echo "gcc make all-gcc failed."
   cd ../../$GCCSRC
   exit 1
fi
make install-gcc
if test $? != 0
then
   echo "gcc make install-gcc failed."
   cd ../../$GCCSRC
   exit 1
fi
cd ../../$GCCSRC

#
# build newlibs
#
#mkdir -p ../build/newlib
#cd ../build/newlib
#../../$GCCSRC/newlib/src/configure --target=propeller-elf --prefix=$PREFIX --enable-target-optspace
#if test $? != 0
#then
#   echo "newlib configure failed."
#   cd ../../$GCCSRC
#   exit 1
#fi
#make all
#if test $? != 0
#then
#   echo "newlib make all failed."
#   cd ../../$GCCSRC
#   exit 1
#fi
#make install
#if test $? != 0
#then
#   echo "newlib make install failed."
#   cd ../../$GCCSRC
#   exit 1
#fi

#
# build cog code and includes
#
cd lib
make clean
if test $? != 0
then
  echo "cog library build failed - make clean"
  cd ..
  exit 1
fi
make PREFIX=$PREFIX ${JOBS} cog
if test $? != 0
then
  echo "cog library build failed - make"
  cd ..
  exit 1
fi
cd ..

#
# build gcc libgcc
# this must be done after the library build, since it depends on
# library header files
#
cd ../build/gcc
make ${JOBS} all-target-libgcc
if test $? != 0
then
   echo "gcc make all-target-libgcc failed"
   cd ../../$GCCSRC
   exit 1
fi
make install-target-libgcc
if test $? != 0
then
   echo "gcc make install-target-libgcc failed."
   cd ../../$GCCSRC
   exit 1
fi
cd ../../$GCCSRC

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
#cp -f ldscripts/* $PREFIX/propeller-elf/lib
#if test $? != 0
#then
#  echo "ldscripts install failed"
#  exit 1
#fi

#
# copy Brad's Spin Tool
#
cp -f release/$SPINCMP $PREFIX/bin
if test $? != 0
then
  echo "openspin install failed"
  exit 1
fi
chmod a+x $PREFIX/bin/$SPINCMP
if test $? != 0
then
  echo "openspin chmod failed"
  exit 1
fi

#
# build gcc libstdc++
# this must be done after the library build, since it depends on
# library header files
#
cd ../build/gcc
make ${JOBS} all
if test $? != 0
then
   echo "gcc libstdc++ make all failed"
   cd ../../$GCCSRC
   exit 1
fi
make install
if test $? != 0
then
   echo "gcc libstdc++ make install failed."
   cd ../../$GCCSRC
   exit 1
fi
cd ../../$GCCSRC

#
# build tiny library
#
cd lib
make clean
if test $? != 0
then
  echo "tiny library build failed - make clean"
  cd ..
  exit 1
fi
make PREFIX=$PREFIX ${JOBS} tiny
if test $? != 0
then
  echo "tiny library build failed - make"
  cd ..
  exit 1
fi
make PREFIX=$PREFIX install-tiny
if test $? != 0
then
  echo "tiny library install failed"
  cd ..
  exit 1
fi
cd ..

#
# build spin2cpp
#
make -C spin2cpp clean
if test $? != 0
then
   echo "spin2cpp make clean failed"
   exit 1
fi

make -C spin2cpp TARGET=$PREFIX BUILDROOT=../../build/spin2cpp
if test $? != 0
then
   echo "spin2cpp make failed"
   exit 1
fi

make -C spin2cpp TARGET=$PREFIX BUILDROOT=../../build/spin2cpp install
if test $? != 0
then
   echo "spin2cpp install failed"
   exit 1
fi

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
../../$GCCSRC/gdb/configure --target=propeller-elf --prefix=${PREFIX} --with-system-gdbinit=${PREFIX}/lib/gdb/gdbinit
if test $? != 0
then
   echo "gdb configure failed"
   cd ../../$GCCSRC
   exit 1
fi

make ${JOBS} all
if test $? != 0
then
   echo "gdb make all failed"
   cd ../../$GCCSRC
   exit 1
fi
if [ ${OS} != "msys" ]
then
    cp -f gdb/gdb ${PREFIX}/bin/propeller-elf-gdb
else
    cp -f gdb/gdb.exe ${PREFIX}/bin/propeller-elf-gdb.exe
fi
mkdir -p ${PREFIX}/lib/gdb

cd ../../$GCCSRC
cp -f gdbstub/gdbinit.propeller ${PREFIX}/lib/gdb/gdbinit

#
# build spinsim
#
cp -r spinsim ../build/.
cd ../build/spinsim
make clean
if test $? != 0
then
   echo "spinsim clean failed"
   cd ../../$GCCSRC
   exit 1
fi
make
if test $? != 0
then
   echo "spinsim make failed"
   cd ../../$GCCSRC
   exit 1
fi
if [ ${OS} != "msys" ]
then
    cp -f spinsim ${PREFIX}/bin/.
else
    cp -f spinsim.exe ${PREFIX}/bin/.
fi
cd ../../$GCCSRC

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
cd ../../$GCCSRC

echo "Build complete."
exit 0
