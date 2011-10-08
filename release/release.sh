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

ARCHIVE=${VERSION}_${BUILDNUM}.tar
echo "Building ${ARCHIVE}"

make -C ../demos clean
tar -c /usr/local/propeller -f ${ARCHIVE}
tar -r ../demos/ -f ${ARCHIVE}
tar -r bstc.linux -f ${ARCHIVE}
gzip ${ARCHIVE}
