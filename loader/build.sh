#!/bin/sh
#
# run this script in this directory (the top level propgcc)
#

#
# ADJUST THE FOLLOWING VARIABLES IF NECESSARY
#
PREFIX=/opt/parallax
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

if test x$UNAME = xDarwin
then
  OS=macosx
  PORT=/dev/cu.usbserial-A8004ILf
  BOARD=hub
elif test x$UNAME = xCygwin
then
  OS=cygwin
  PORT=COM16
  BOARD=c3
elif test x$UNAME = xMsys
then
  OS=msys
  PORT=COM16
  BOARD=c3
elif test x$UNAME = xLinux
then
  OS=linux
  PORT=/dev/ttyUSB0
  BOARD=c3
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
# build propeller-load
#
make TARGET=../../../build/loader clean
make TARGET=../../../build/loader
make TARGET=../../../build/loader install
echo "cp ./bin/${OS}/* ${PREFIX}/bin/."
cp   ./bin/${OS}/* ${PREFIX}/bin/.
ls -l ${PREFIX}/bin/bin2c*
ls -l ${PREFIX}/bin/propeller-load*
echo "Build complete."
exit 0
