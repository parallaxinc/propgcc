#!/bin/sh

#
# This script creates a demos zip/tarball
#
# Copyright (c) 2011 by Parallax, Inc.
# Code by Steve Denson
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

NAME=demos
VERSION=`cat VERSION.txt | grep -v "^#"`
ARCHIVE=${VERSION}_${NAME}
PACKAGE=../demos
echo "Building ${PACKAGE}"

find ${PACKAGE} -name *.dat | xargs rm -vf
find ${PACKAGE} -name *.elf | xargs rm -vf
find ${PACKAGE} -name *.list | xargs rm -vf
find ${PACKAGE} -name *.o | xargs rm -vf
find ${PACKAGE} -name *.out | xargs rm -vf
find ${PACKAGE} -name foo* | xargs rm -vf

rm -f ${ARCHIVE}.zip
cp -r ${PACKAGE} .
zip ${ARCHIVE} -r demos
rm -rf demos

exit 0
