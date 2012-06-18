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
doxygen
if test $? != 0
then
  echo "doxygen failed. installed?"
  cd ..
  exit 1
fi
mkdir -p /opt/parallax/share/lib
if test $? != 0
then
  echo "mkdir /opt/parallax/share/lib failed"
  cd ..
  exit 1
fi
cp -r html /opt/parallax/share/lib
if test $? != 0
then
  echo "cp -r html /opt/parallax/share/lib failed"
  cd ..
  exit 1
fi

