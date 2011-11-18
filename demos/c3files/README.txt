c3files is a test program that reads and writes files on an SD card using the
Propeller C3 card.  The test program can be built as either an LMM, XMM or
XMMC program.  This is done by entering the appropriate subdirectory and
executing the make command.

After the program is built it can be executed by entering a "make run"
command.  The environmental variable, PROPELLER_LOAD_PORT must by set to the
com port that the C3 card is attached to.

The test program implements basic cat, echo, rm, ls, cd, pwd, mkdir and
rmdir  commands.  It also implements file redirection by using the "<", ">"
and ">>" operatiors.  The cat command can be used to create and type out'
files.  When cat reads from the standard input, the input is terminated by
typing a control-D followed immediately by a carriage return.

The ls command will list out the file names in the root directory or any
subdirectories that are included in the command line.  A long directory
listing is obtained by using the -l option, such as "ls -l".  The ll
command can be used insted of ls -l.

The following shows examples of how the commands can be used.

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

Note, other SD cards can be used in the LMM mode by specifying the pins
for data out, clock, data in and chip select.  The pin assignments for
the Spinneret card are defined in filetest.c.  This can be modified to
handle other pin assignments.
