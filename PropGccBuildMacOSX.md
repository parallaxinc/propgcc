## Mac OS X Build Procedure ##

> Building propgcc on OS X Lion 10.7:

> ### Prerequisite ###

> Mercurial, the free distributed source control management tool from: http://mercurial.selenic.com/downloads/. This is needed to download and maintain the propgcc sources. Mercurial is not available in OS X as a default and must be downloaded and installed as a package. Download Mercurial's ".mpkg" (meta-package) file and double-click on it to install. The command for Mercurial is "hg"...


> Note: Executing the "hg clone" command on my 10.7.2 system failed with "AttributeError: 'module' object has no attribute 'setbinary'". If this happens, you'll need to edit the file: "/usr/local/bin/hg", according to instructions on this [web page](http://groups.google.com/group/thg-dev/browse_thread/thread/dbbce627480a5a13?pli=1)


> ### Setup Steps ###

> Mac OS X Lion 10.7 addendum to: PropGccBuild - propgcc - Describes how to build Propeller GCC. - GCC for the Parallax Propeller Microcontroller - [Google Project](http://code.google.com/p/propgcc/)
```
1. Make a new directory for the propgcc sources, in a known location (your home directory is a good place).
```
```
$ cd $HOME      <== go to your home directory
$ mkdir propSrc <== create a new directory for your cloned propgcc sources
```
```
2. You will need to add the following lines to your ".profile" file in your home directory.
   Use nano, vi, emacs or any GUI editor you like.
```
```
# propgcc Programming PATH
export PATH=${PATH}:/opt/parallax/bin
```
```
# Set up Python to help mercurial
export PYTHONPATH=${HOME}/lib/python
export PATH=${HOME}/bin:$PATH

3. Be sure to "source" .profile before continuing...

$ source .profile
```
```
4. To download the propgcc sources, you will need to use Mercurial's 'clone' command: "hg clone".
   If the command fails, see the above note for instructions on repairing the failure.
   The cloning process may take some time, so be prepared to wait if Mercurial appears to stall for a while...
```
```
$ hg clone --uncompressed https://code.google.com/p/propgcc/ <== clone propgcc sources to your source directory (3-20 minutes)
$ cd propgcc <== go into the cloned propgcc directory
$ ls <== list the directory content
  LICENSE.txt dejagnu gcc jbuild.sh loader release
  README.txt demos gdb ldscripts newlib tools
  binutils doc gnu-oids.texi lib rebuild.sh
```

> ### Build propgcc on OS X Lion ###
```
1. $ sudo rm -rf /opt/parallax <== work as root user without being dangerous (requires admin password)
2. $ sudo mkdir /opt/parallax
3. $ sudo chown ${USER} /opt/parallax <== makes the new directory accessible to you
4. $ cd $HOME/propSrc/propgcc <== in case you have moved to another directory since the hg clone
5. $ ./rebuild.sh <== or, use "./jbuild.sh N", where "N" is the number of cores to use for make
      (speeds up the build)
6. Building the sources will take quite some time. Go have a cup of espresso (you will need the caffeine)
```

> ### Example Build of the c3files program ###

> On OS X, the USB to Serial connection may show up like this: "/dev/cu.usbserial-0000nnaa", where "nn" will be a two-digit number and "aa" will be a two character alpha. This is the serial number of your propeller board. With the USB cable plugged into the CPU and a C3, executing "ls /dev|grep cu.usbserial" will display the correct name. You'll need to re-export if you plug the cable into a different USB port or new propeller board.
```
$ ls /dev|grep cu.usbserial <== find out what OS X is calling your USB to Serial connection

cu.usbserial-000013FD <== export this as PROPELLER_LOAD_PORT (will change for each board)
```
```
1. $ export PROPELLER_LOAD_PORT=/dev/cu.usbserial-000013FD
2. $ cd $HOME/propsource/propgcc/demos/c3files/lmm
3. $ make
4. $ make run
propeller-load -b c3 c3files.elf -r -t
Propeller Version 1 on /dev/cu.usbserial-000013FD
Writing 30756 bytes to Propeller RAM.
Verifying ... Upload OK!
[ Entering terminal mode. Type ESC or Control-C to exit. ]


Commands are help, cat, rm, ls, ll, echo, cd, pwd, mkdir and rmdir
>
```

Instructions contributed by dgately

I'm responsible for wiki translation errors.
--Steve.