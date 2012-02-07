#
# build library
#
gcc -o remove.exe -Wall remove.c
if test $? != 0
then
  echo "tool build remove.exe failed"
  cd ..
  exit 1
fi

cd make-3.81
./configure
if test $? != 0
then
  echo "tools build failed - configure make"
  cd ..
  exit 1
fi
make
if test $? != 0
then
  echo "tools build failed - make"
  cd ..
  exit 1
fi
cd ..

exit 0
