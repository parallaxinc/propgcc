#!/bin/sh

#
# This script creates a propeller-gcc linux release tarball
#
# Copyright (c) 2011 by Parallax, Inc.
# Code by Steve Denson
# MIT Licensed
#

VERSION=`cat VERSION.txt | grep -v "^#"`
VERFILE=${VERSION}.txt
DATECMD=`date '+%Y-%m-%d-%H:%M:%S'`

#
# ADJUST THE FOLLOWING VARIABLES IF NECESSARY
#
PREFIX=/usr/local/propeller
export PREFIX

#
# attempt to auto-detect the OS
#
UNAME=`uname -s`
if test NAME`echo $UNAME | grep "[_-]"` != NAME
then
  # if system has [-_] try uname -o ... Cygwin for example
  UNAME=`uname -o`
fi
echo "OS '$UNAME' detected."

#
# SHOW VERSION INFO AND GET/SET NEW VERSION
#
echo ${VERFILE}

if [ -w ${VERFILE} ]; then
  echo "Read Version File " ${VERFILE}
  LINE=`tail -n 1 ${VERFILE}`
  BUILDNUM=`echo ${LINE} | cut -d " " -f1`
  BUILDNUM=`expr ${BUILDNUM} + 1`
else
  echo "New Version File " ${VERFILE}
  LINE="0 ${DATECMD}"
  BUILDNUM=0
fi

echo "${BUILDNUM} ${DATECMD}" >> ${VERFILE} 
tail ${VERFILE}

ARCHIVE=${VERSION}_${BUILDNUM}
MACH=`uname -m`
PACKAGE=${MACH}-${ARCHIVE}
echo "Building ${PACKAGE}"

rm -rf propgcc
mkdir ${PACKAGE}

make -C ../demos clean
cp -r ../demos ${PACKAGE}/.
cp ../gcc/COPYING* ${PACKAGE}/.
cp LICENSE.txt ${PACKAGE}/.
cp -r /usr/local/propeller/* ${PACKAGE}/.

# To make a C only release use: ./release.sh C
if test ARG$1 == ARGC
then
  echo "Removing C++ libraries from C Package"
  rm -rv ${PACKAGE}/bin/propeller-elf-c++*
  rm -rv ${PACKAGE}/bin/propeller-elf-g++*
  rm -rv ${PACKAGE}/libexec/gcc/propeller-elf/4.6.1/cc1plus.exe
  rm -rv ${PACKAGE}/propeller-elf/bin/c++*
  rm -rv ${PACKAGE}/propeller-elf/bin/g++*
  rm -rv ${PACKAGE}/propeller-elf/include/c++
  rm -rv ${PACKAGE}/propeller-elf/lib/libstdc++*
  rm -rv ${PACKAGE}/propeller-elf/lib/libsupc++*
  rm -rv ${PACKAGE}/propeller-elf/lib/short-doubles/libstdc++*
  rm -rv ${PACKAGE}/propeller-elf/lib/short-doubles/libsupc++*
  rm -rv ${PACKAGE}/propeller-elf/lib/xmm/libstdc++*
  rm -rv ${PACKAGE}/propeller-elf/lib/xmm/libsupc++*
  rm -rv ${PACKAGE}/propeller-elf/lib/xmm/short-doubles/libstdc++*
  rm -rv ${PACKAGE}/propeller-elf/lib/xmm/short-doubles/libsupc++*
  rm -rv ${PACKAGE}/propeller-elf/lib/xmmc/libstdc++*
  rm -rv ${PACKAGE}/propeller-elf/lib/xmmc/libsupc++*
  rm -rv ${PACKAGE}/propeller-elf/lib/xmmc/short-doubles/libstdc++*
  rm -rv ${PACKAGE}/propeller-elf/lib/xmmc/short-doubles/libsupc++*
fi

if test x$UNAME = xDarwin
then
  ARCHIVE=macosx-${PACKAGE}.tar

  mkdir ${PACKAGE}/usr
  mkdir ${PACKAGE}/usr/local
  mv ${PACKAGE}/propeller ${PACKAGE}/usr/local/.

  cp bstc.osx ${PACKAGE}/${PREFIX}/bin
  cp bstc.osx ${PACKAGE}/${PREFIX}/bin/bstc
  tar -c ${PACKAGE} -f ${ARCHIVE}
  gzip ${ARCHIVE}

elif test x$UNAME = xCygwin
then
  ARCHIVE=cygwin-${PACKAGE}.zip
  mv ${PACKAGE}/propeller ${PACKAGE}/usr/local/.

  cp bstc.exe ${PACKAGE}/bin
  cp INSTALL.txt ./${PACKAGE}
  cp README_CYGWIN.txt ./${PACKAGE}
  mv ${PACKAGE} propgcc
  zip ${ARCHIVE} -r propgcc

elif test x$UNAME = xMsys
then
  ARCHIVE=windows-${PACKAGE}.zip
  # propeller directory already copied to package

  cp bstc.exe ./${PACKAGE}/bin
  cp PropGCC.bat ./${PACKAGE}
  cp INSTALL.txt ./${PACKAGE}
  cp README_WINDOWS.txt ./${PACKAGE}
  cp ../tools/make-3.81/make.exe ./${PACKAGE}/bin
  cp ../tools/remove.exe ./${PACKAGE}/bin/rm.exe
  mv ${PACKAGE} propgcc
  zip ${ARCHIVE} -r propgcc

elif test x$UNAME = xLinux
then
  ARCHIVE=linux-${PACKAGE}.tar

  mkdir ${PACKAGE}/usr
  mkdir ${PACKAGE}/usr/local
  mv ${PACKAGE}/propeller ${PACKAGE}/usr/local/.

  cp bstc.linux ${PACKAGE}/${PREFIX}/bin
  cp bstc.linux ${PACKAGE}/${PREFIX}/bin/bstc
  cp INSTALL.txt ${PACKAGE}
  tar -c ${PACKAGE} -f ${ARCHIVE}
  gzip ${ARCHIVE}

else
  echo "Unknown system: " $UNAME
  exit 1
fi

