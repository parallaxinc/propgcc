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
PACKROOT=./propgcc
echo "Building ${PACKAGE}"

rm -rf ${PACKROOT}
mkdir -p ${PACKROOT}

# demos are now in a separate package
#make -C ../demos clean
#cp -r ../demos ${PACKROOT}/

cp ../gcc/COPYING* ${PACKROOT}/.
cp ../LICENSE.txt ${PACKROOT}/.

#
# copy in the basic build tree
#
cp -r ${PREFIX}/* ${PACKROOT}/

#
# move docs
#
mkdir -p ${PACKROOT}/doc
cp -r ../doc/* ${PACKROOT}/doc
if [ -d ${PACKROOT}/share/doc/gcc ]
then
  mv ${PACKROOT}/share/doc/gcc ${PACKROOT}/doc
fi

#
# remove cruft
#

rm -r ${PACKROOT}/share

#
# now make the packages
#
if test x$UNAME = xDarwin
then
  ARCHIVE=macosx-${PACKAGE}.tar

  cp bstc.osx ${PACKROOT}/bin
  cp bstc.osx ${PACKROOT}/bin/bstc
  tar -c ${PACKAGE} -f ${ARCHIVE}
  gzip ${ARCHIVE}

elif test x$UNAME = xCygwin
then
  ARCHIVE=cygwin-${PACKAGE}.zip

  cp bstc.exe ${PACKROOT}/bin
  cp INSTALL.txt ${PACKROOT}/
  cp README_CYGWIN.txt ${PACKROOT}/
  mv ${PACKAGE} ${PACKROOT}
  zip ${ARCHIVE} -r ${PACKROOT}

elif test x$UNAME = xMsys
then
  ARCHIVE=windows-${PACKAGE}.zip

  cp bstc.exe ${PACKROOT}/bin
  cp addpath.bat ${PACKROOT}
  cp PropGCC.bat ${PACKROOT}
  cp INSTALL.txt ${PACKROOT}
  cp README_WINDOWS.txt ${PACKROOT}
  cp ../tools/make-3.81/make.exe ${PACKROOT}/bin
  cp libiconv-2.dll ${PACKROOT}/bin
  cp libintl-8.dll ${PACKROOT}/bin
  cp ../tools/remove.exe ${PACKROOT}/bin/rm.exe
  zip ${ARCHIVE} -r ${PACKROOT}

elif test x$UNAME = xLinux
then
  ARCHIVE=linux-${PACKAGE}.tar

  cp bstc.linux ${PACKROOT}/bin
  cp bstc.linux ${PACKROOT}/bin/bstc
  cp INSTALL.txt ${PACKROOT}
  mv ${PACKROOT} propeller
  tar -c ./propeller -f ${ARCHIVE}
  gzip ${ARCHIVE}

else
  echo "Unknown system: " $UNAME
  exit 1
fi

