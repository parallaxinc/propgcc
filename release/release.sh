#!/bin/sh -x

#
# This script creates a propeller-gcc linux release tarball
#
# Copyright (c) 2011-2012 by Parallax, Inc.
# Code by Steve Denson
# MIT Licensed
#

#
# We no longer use the propgcc_v* update system. June 2012
#

VERSION=`cat VERSION.txt | grep -v "^#"`
DATECMD=`date '+%Y-%m-%d-%H:%M:%S'`

#
# ADJUST THE FOLLOWING VARIABLES IF NECESSARY
#
PREFIX=/opt/parallax
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
echo ${VERSION}

ARCHIVE=${VERSION}
MACH=`uname -m`
PACKAGE=${ARCHIVE}-${MACH}
PACKROOT=parallax
echo "Building ${PACKAGE}"

if test x$UNAME = xMsys
then
    PACKROOT=propgcc
elif test x$UNAME = xCygwin
then
    PACKROOT=propgcc
fi


rm -rf ${PACKROOT}
mkdir -p ${PACKROOT}

# demos are now in a separate package
#make -C ../demos clean
#cp -r ../demos ${PACKROOT}/

cp ../gcc/COPYING* ${PACKROOT}/.
cp ../LICENSE.txt ${PACKROOT}/.

echo "Propeller GCC Version ${BUILDNUM} build time ${DATECMD}" >> ${PACKROOT}/${ARCHIVE}.txt

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
  ARCHIVE=${PACKAGE}-macosx.tar.bz2

  cp bstc.osx ${PACKROOT}/bin
  cp bstc.osx ${PACKROOT}/bin/bstc
  cp INSTALL.txt ${PACKROOT}
  tar -cjf ${ARCHIVE} ${PACKROOT}

elif test x$UNAME = xCygwin
then
  ARCHIVE=${PACKAGE}-cygwin.zip

  cp bstc.exe ${PACKROOT}/bin
  cp INSTALL.txt ${PACKROOT}/
  cp README_CYGWIN.txt ${PACKROOT}/
  mv ${PACKAGE} ${PACKROOT}
  zip ${ARCHIVE} -r ${PACKROOT}

elif test x$UNAME = xMsys
then
  ARCHIVE=${PACKAGE}-windows.zip

  cp bstc.exe ${PACKROOT}/bin
  cp addpath.bat ${PACKROOT}
  cp PropGCC.bat ${PACKROOT}
  cp INSTALL.txt ${PACKROOT}
  cp README_WINDOWS.txt ${PACKROOT}
  cp ../tools/make-3.81/make.exe ${PACKROOT}/bin
  cp libiconv-2.dll ${PACKROOT}/bin
  cp libintl-8.dll ${PACKROOT}/bin
  cp libgcc_s_dw2-1.dll ${PACKROOT}/bin
  cp ../tools/remove.exe ${PACKROOT}/bin/rm.exe
  zip ${ARCHIVE} -r ${PACKROOT}

elif test x$UNAME = xLinux
then
  ARCHIVE=${PACKAGE}-linux.tar.bz2

  cp bstc.linux ${PACKROOT}/bin
  cp bstc.linux ${PACKROOT}/bin/bstc
  cp INSTALL.txt ${PACKROOT}
  tar -cj ${PACKROOT} -f ${ARCHIVE}

else
  echo "Unknown system: " $UNAME
  exit 1
fi

