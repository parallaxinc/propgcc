# Building Propeller GCC #

> Today we can build everything on Linux, `*`Mac OSX, and Windows Cygwin with the build procedure. Many of these instructions are derived from the propgcc README.txt http://code.google.com/p/propgcc/source/browse/README.txt

> There are prerequisites for building Propeller GCC, you can find them on the [prerequisites](PropGccBuildPreReq.md) page.

> The [Openspin](https://github.com/reltham/OpenSpin) compiler created by RoyEltham based on Chip's code now replaces BSTC. The Openspin source is available on github: https://github.com/reltham/OpenSpin

> Parallax and the Propeller-GCC team greatly appreciate the work done by Brad Campbell on BSTC which can be used with Propeller-GCC.  BSTC is not necessary for compiling C programs with Propeller GCC, but may be used in demos. For now BSTC is used for compiling items in the loader, but the loader is not normally compiled by users. BSTC is distributed with Propeller-GCC with written consent from Brad Campbell. BSTC and friends are supported via the BST web site http://www.fnarfbargle.com/bst.html

**Important**: If your build fails with something like:
```
../../../../propgcc/binutils/bfd/doc/bfd.texinfo:325: unknown command `colophon`
../../../../propgcc/binutils/bfd/doc/bfd.texinfo:336: unknown command `cygnus`
```
Then you need to downgrade texinfo. Currently, texinfo 5.0 and above is not supported. Instructions for downgrading are [here](http://forums.parallax.com/showthread.php/151005-PropGCC-build-fails-on-Ubuntu-13.10?p=1216485&viewfull=1#post1216485).


## Build Propeller GCC on Linux or Cygwin ##

> ### Download the source ###
    1. Open a command window
    1. Create a folder for the project
    1. `cd` to your folder
    1. `$ cd "your folder"`
    1. `$ hg clone https://code.google.com/p/propgcc/ propgcc`
    1. `$ cd propgcc`

> Please add /opt/parallax/bin to your path:

> `$ PATH=${PATH}:/opt/parallax/bin`

> Alternatively, you can add it to your path by appending the following line to your ~/.bashrc file, and restarting your terminal:
> `export PATH=/opt/parallax/bin:${PATH}`

> Please download and build [OpenSpin](https://github.com/reltham/OpenSpin) and put it in your PATH.

> ### Build PropellerGCC ###
> Please run the following commands of from the root of your local copy of the propgcc repository.

> #### Linux/Mac ####
    1. Set a GROUP variable.
    1. `su root` (or use sudo for the following commands through "exit root")
    1. `$ export GROUP="your group"`
    1. `$ rm -rf /opt/parallax`
    1. `$ mkdir /opt/parallax`
    1. `$ chown ${USER}.${GROUP} /opt/parallax`
    1. `$ chmod g+w /opt/parallax`
    1. `exit root`

> For release\_1\_0 branch
    1. `$ ./jbuild.sh 6 rm-all`
    1. Go have a cup of coffee (build takes 15 minutes on I7 3.3MHz).

> For default branch
    1. $ `make clean-all`
    1. $ `make`
    1. $ Go eat lunch.

> #### Windows Cross Compile ####
> For default branch cross-compile to Windows
    1. $ `make clean-all`
    1. $ `make`
    1. $ `make CROSS=win32`
    1. $ Go eat lunch.

> #### RPi ARM Cross Compile ####
> For default branch cross-compile to RPi
    1. $ `make clean-all`
    1. $ `make`
    1. $ `make CROSS=rpi`
    1. $ Go eat lunch.

> #### Ubuntu ####
    1. `$ sudo rm -rf /opt/parallax`
    1. `$ sudo mkdir /opt/parallax`
    1. `$ sudo chown ${USER}.${USER} /opt/parallax`
    1. `$ sudo chmod g+w /opt/parallax`

> For release\_1\_0 branch
    1. `$ ./jbuild.sh 6 rm-all`
    1. Go have a cup of coffee (build takes 15 minutes on I7 3.3MHz).

> For default branch
    1. $ `make clean-all`
    1. $ `make`
    1. $ Go eat lunch.

> Note for Linux users: you may need to add your user to group `dialout` to access the FTDI USB port: `$ sudo adduser ${USER} dialout`


> #### Windows Cygwin ####
    1. `$ ./rebuild.sh`
    1. Take the day off.

> #### Windows Msys/Mingw ####
> For release\_1\_0 branch
    1. `$ ./rebuild.sh`
    1. Take the day off.


## Mac OS X Build Procedure ##

> Visit the [Mac OS X page](PropGccBuildMacOSX.md) for more detailed information.
> Note `*`Mac OSX users with Xcode version < 4.2 must use "native" GCC for builds instead of LLVM. Propeller GCC will not build with LLVM on Xcode version 4.1. Xcode 4.2 users can build with either GCC or LLVM.

## MinGW Build Procedure ##
> Propeller-GCC uses MSYS and MinGW to build the tool-chain. MinGW is a free Minimalist Windows interface for GNU based programs. Unmodified MinGW binary tools and binary libraries can be used without any GNU licensing restrictions whatsoever.
> MSYS is a minimalist operating system environment for building programs like the Propeller-GCC toolchain with MinGW. The MinGW build procedure requires that the Windows user install MSYS and MinGW. Please install MSYS and MinGW as top folders in your drive such as C:\MSYS and C:\MinGW using the latest installer provided by at  http://sourceforge.net/projects/mingw/files/Installer/mingw-get-inst/ MSYS and MinGW must be in your %PATH% for the build to succeed.

> MSYS IS NOT REQUIRED FOR BUILDING PROPELLER PROGRAMS. MinGW binaries are distributed with Propeller-GCC according to terms described here: http://www.mingw.org/license These terms are free in every sense of the word with no strings attached for MinGW binaries.

> #### Once you have MSYS and MinGW installed ####
> > Start a MinGW shell from the Start menu. Your shell prompt may look different but we use $ in the following to indicate the prompt; don't type the $ that's shown at the beginning of the commands.
> > Add a path to Mercurial, and /opt/parallax/bin:
      * $ PATH="/opt/parallax/bin:/c/Program Files/TortoiseHg:$PATH"
> > Create a folder for your project with no spaces such as gccdev
      * $ cd
      * $ mkdir gccdev
      * $ cd gccdev
> > Get the repository:
      * $ hg clone https://code.google.com/p/propgcc/ propgcc
> > Getting the repository will take a long time. Sometime the clone will fail, and you will have to try again until it succeeds.
> > Once you have the repository:
      * $ cd propgcc
      * $ source fix-xcode-warnings.sh
> > Now start the release\_1\_0 branch build using the rebuild.sh script.
      * $ ./rebuild.sh
> > Use make for default branch builds.
      * $ make clean-all
      * $ make
> > Rebuild from scratch takes about 40 minutes on a 3.3GHz CPU. A multiple job rebuild (jbuild.sh) may fail.
> > When rebuild completes without errors, you will find /opt/parallax fully populated.


> #### Using Propeller-GCC after build ####
> > The MinGW build from the Windows command DOS window perspective will be in C:\MSYS\local\propeller. You can copy this folder anywhere you want and use it to compile and load Propeller-GCC programs from the command window as long as your DOS %PATH% has your directory with bin in it. For example:
    1. Use Windows Explorer to copy the contents of C:\MSYS\local\propeller to C:\propeller-gcc.
    1. C:\>set PATH=C:\propeller-gcc\bin;%PATH%
    1. C:\>cd gccdev\propgcc\demos
    1. C:\gccdev\propgcc>make


> #### Creating a MinGW Propeller-GCC distribution ####
> TBD


## Build the .pdf Documentation ##
> Once you have built Propeller GCC, you can generate .pdf documentation. The TexLive or similar program will be needed for this.
    * $ cd propgcc/../build/gcc
    * $ make pdf
    * $ make install-pdf
> Documentation build is not part of the propgcc/rebuild.sh script mainly as a convenience for developers. The .pdf documentation can be posted separately for release. Files /opt/parallax/share/doc/gcc/cpp.pdf and /opt/parallax/share/doc/gcc/gcc.pdf are the compiler documents.

## Build the Loader ##
> Once you have installed Propeller GCC you should build and configure the loader.
    * The loader lets you download applications.
    * The loader is explained in [Propeller GCC Loader](PropGccLoader.md).
    * There are some demonstration programs in [propgcc/demos](PropGccDemos.md).



# Build Scripts #

> Often changes are made that require removing the target build folder and building from scratch. This happens less as time passes, but if you see a build error that is not reported at the main page, it is likely that you will need to rebuild everything.

> The jbuild.sh and rebuild.sh script that comes with propgcc will build everything from scratch. If you want to build without deleting directories, just use $ ./jbuild.sh 6 rm-all or $ ./rebuild.sh rm-all

The rebuild.sh script calls jbuild.sh 1