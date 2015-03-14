The current trunk GCC build prerequisites are listed here: http://gcc.gnu.org/install/prerequisites.html

What follows on this page are the build requisites that are necessary to build PropellerGCC.

## Linux Build Requirements ##

  * The best Linux for building Propeller GCC today is Debian Linux. Ubuntu (tested with 12.04, 12.10, and 13.04) works fine as well.
  * Use VirtualBox on Windows and install Debian if you like.
  * ARM based distributions can be built on the host (RaspberryPi, etc.).
  * Windows can be cross-compiled on Linux with the mingw32 package. make CROSS=win32


## Necessary Packages ##

Once you have a good Linux distribution, some things must be added. If you look at the main GCC Prerequisites page, you will note an overwhelming number of items. As a minimum for PropGCC, you should install:

  * Mercurial source control
  * GNU make version 3.80 (or later)
  * autoconf version 2.64
  * automake version 1.11.1
  * Bison
  * Flex version 2.5.4 (or later)
  * gettext version 0.14.5 (or later)
  * GNU m4 version 1.4.6 (or later)
  * Perl version 5.6.1 (or later)
  * Texinfo version 4.7 (or later) (version 4.13 for Ubuntu 13.10 ... see below)
  * Ncurses development files for termcap (try "libncurses-dev" or "libncurses5-dev")
  * expat-dev (try "libexpat1-dev")

Items in our repository. Please don't add these separately:

  * GNU binutils
  * MPC Library version 0.8.1
  * MPFR Library version 2.4.2
  * GNU Multiple Precision Library (GMP) version 4.3.2 (or later) - see Prerequisites!

Other items:

  * DejaGnu 1.4.4 - test suite stuff only
  * Expect - test suite stuff only
  * Tcl - test suite stuff only
  * TexLive - for creating pdf documentation

Building on Ubuntu 13.10 requires texinfo4.13 using these instructions:
```
wget http://ftp.gnu.org/gnu/texinfo/texinfo-4.13a.tar.gz
tar -zxvf texinfo-4.13a.tar.gz
cd texinfo-4.13
./configure
make
sudo make install

makeinfo --help
```

See the trunk GCC Prerequisites link for more details. Many of the other libraries in the Prerequisites page seem to be installed by default in most distributions.

## Installing with VirtualBox ##

  1. Download debian-6.0.2.1-i386-netinst
> (Later version of debian may be necessary for mingw cross-compiler builds - see comments below.)
  1. Make a CD with the distribution
  1. Install VirtualBox on the PC
  1. Install debian from CD booted in VirtualBox
  1. Use root access or sudo for the following
  1. $ apt-get install mercurial
  1. $ apt-get install flex
  1. $ apt-get install bison
  1. $ apt-get install autoconf
  1. $ apt-get install gettext
  1. $ apt-get install texlive
  1. $ apt-get install texinfo (if missing)
  1. $ apt-get install ncurses-dev

Other packages seemed to be there already except for GMP, etc...
GMP is committed to the propgcc repository.

If you want to use VirtualBox Linux to download programs to a Propeller chip, you will need to enable the device in the VirtualBox Devices menu. Also, when accessing the serial port, you will need to use $ sudo adduser $USER dialout .... Then reboot.