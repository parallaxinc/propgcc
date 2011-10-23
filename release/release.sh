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
echo "Building ${ARCHIVE}"

make -C ../demos clean

if test x$UNAME = xDarwin
then
  OS=macosx

elif test x$UNAME = xCygwin
then

  NAME=propgcc
  ARCHIVE=windows-x86-32-$ARCHIVE.zip
  rm -f $ARCHIVE
  rm -rf $NAME
  mkdir $NAME
  mkdir ${NAME}/bin
  mkdir ${NAME}/usr
  mkdir ${NAME}/usr/local
  cp -r ../demos ./${NAME}/
  cp -r /usr/local/propeller ./${NAME}/usr/local/.
  cp bstc.exe ./${NAME}/bin
  cp /bin/cyggcc_s-1.dll ./${NAME}/bin
  cp /bin/cygiconv-2.dll ./${NAME}/bin
  cp /bin/cygintl-8.dll ./${NAME}/bin
  cp /bin/cygncursesw-10.dll ./${NAME}/bin
  cp /bin/cygreadline7.dll ./${NAME}/bin
  cp /bin/cygwin1.dll ./${NAME}/bin
  cp /bin/cygz.dll ./${NAME}/bin
  cp /bin/echo.exe ./${NAME}/bin
  cp /bin/make.exe ./${NAME}/bin
  cp /bin/rm.exe ./${NAME}/bin
  cp /bin/sh.exe ./${NAME}/bin
  cp addpath.bat ./${NAME}
  cp INSTALL.txt ./${NAME}
  cp README_WINDOWS.txt ./${NAME}
  zip $ARCHIVE -r $NAME

elif test x$UNAME = xLinux
then

  ARCHIVE=windows-x86-32-${ARCHIVE}.tar
  tar -c /usr/local/propeller -f ${ARCHIVE}
  tar -r ../demos/ -f ${ARCHIVE}
  tar -r bstc.linux -f ${ARCHIVE}
  tar -r INSTALL.txt -f ${ARCHIVE}
  gzip ${ARCHIVE}

else
  echo "Unknown system: " $UNAME
  exit 1
fi

