filetest is a test program that reads and writes files on an SD card.  The test
program can be built as either a CMM, LMM, XMM or XMMC program.  This is done
by specifing the model using make as follows:

make MODEL=xmmc

The default model is cmm.  When changing models you must do a "make clean"
before building with the new model.

After the program is built it can be executed by entering a "make run"
command.  The environmental variable, PROPELLER_LOAD_BOARD must by set when
using the xmm or xmmc memory model.  It is also required for the cmm and lmm
models when automatically configuring the SD card pins through the loader.

The filetest program can also be build using SimpleIDE with the filetest.side
project file.

The filetest program implements basic cat, echo, rm, ls, cd, pwd, mkdir and
rmdir  commands.  It also implements file redirection by using the "<", ">"
and ">>" operatiors.  The cat command can be used to create and type out'
files.  When cat reads from the standard input, the input is terminated by
typing a control-D followed immediately by a carriage return.

The ls command will list out the file names in the root directory or any
subdirectories that are included in the command line.  A long directory
listing is obtained by using the -l option, such as "ls -l".  The ll
command can be used insted of ls -l.

The following example shows how the commands can be used.

cat >file1
cat file1 >file2
cat file1 file2 >file3
ls
ls -l
ls directory
echo This is a test. >file4
echo This is line 2. >>file4
rm file1 file2 file3 file4
mkdir testdir
pwd
cd testdir
cd ..
rmdir testdir

Note, the file system is automatically initialized by the loader by using the
appropriate CFG file.  The mount routine can also be used to define the SD card
pin numbers and to mount the file system.  This is done by defining CALL_MOUNT
in filetest.c and setting the pin numbers in the mount function.  See the mount
function for examples of setting the pin numbers for various types of cards.
