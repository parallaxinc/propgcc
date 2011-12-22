#!/bin/sh

#
# This script creates a library source code zip
#
# Copyright (c) 2011 by Parallax, Inc.
# Code by Steve Denson & Eric Smith
# MIT Licensed
#

DATECMD=`date '+%Y-%m-%d'`

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

NAME=propgcc-libsrc
ARCHIVE=${NAME}_${DATECMD}
PACKAGE=../lib
echo "Building ${PACKAGE}"

find ${PACKAGE} -name *.dat | xargs rm -vf
find ${PACKAGE} -name *.elf | xargs rm -vf
find ${PACKAGE} -name *.list | xargs rm -vf
find ${PACKAGE} -name *.o | xargs rm -vf
find ${PACKAGE} -name *.a | xargs rm -vf
find ${PACKAGE} -name *.out | xargs rm -vf
find ${PACKAGE} -name foo* | xargs rm -vf

rm -f ${ARCHIVE}.zip
cp -r ${PACKAGE} .
zip ${ARCHIVE} -r lib
rm -rf lib

exit 0

