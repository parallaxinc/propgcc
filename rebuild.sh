#!/bin/sh

if test x$1 = x
then
  ./jbuild.sh 1 rm
else
  ./jbuild.sh 1
fi
