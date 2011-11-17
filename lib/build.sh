#
# build library
#
make clean
if test $? != 0
then
  echo "library build failed - make clean"
  cd ..
  exit 1
fi
make
if test $? != 0
then
  echo "library build failed - make"
  cd ..
  exit 1
fi
make install
if test $? != 0
then
  echo "library install failed"
  cd ..
  exit 1
fi

