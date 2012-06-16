/*
############################################################################
# This program is used to test the basic functions of the SD file system.
# It implements simple versions of the cat, rm, ls, echo, cd, pwd, mkdir and
# rmdir commands plus the <, > and >> file redirection operators.
# The program starts up the file driver and then prompts for a command.
#
# Written by Dave Hein
# Copyright (c) 2011 Parallax, Inc.
# MIT Licensed
############################################################################
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cog.h>
#include <ctype.h>
#include <propeller.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/sd.h>

extern _Driver _SimpleSerialDriver;
extern _Driver _FileDriver;

/* This is a list of all drivers we can use in the
 * program. The default _InitIO function opens stdin,
 * stdout, and stderr based on the first driver in
 * the list (the serial driver, for us)
 */
_Driver *_driverlist[] = {
  &_SimpleSerialDriver,
  &_FileDriver,
  NULL
};

char *FindChar(char *ptr, int val);

FILE *stdinfile;
FILE *stdoutfile;

/* Print help information */
void Help()
{
    printf("Commands are help, cat, rm, ls, ll, echo, cd, pwd, mkdir and rmdir\n");
}

void Cd(int argc, char **argv)
{
    if (argc < 2) return;

    if (chdir(argv[1]))
        perror(argv[1]);
}

void Pwd(int argc, char **argv)
{
    uint8_t buffer[64];
    char *ptr = getcwd(buffer, 64);
    if (!ptr)
        perror(0);
    else
        fprintf(stdoutfile, "%s\n", ptr);
}

void Mkdir(int argc, char **argv)
{
    int i;

    for (i = 1; i < argc; i++)
    {
        if (mkdir(argv[i], 0))
            perror(argv[i]);
    }
}

void Rmdir(int argc, char **argv)
{
    int i;

    for (i = 1; i < argc; i++)
    {
        if (rmdir(argv[i]))
            perror(argv[i]);
    }
}

/* This routine implements the file cat function */
void Cat(int argc, char **argv)
{
    int i;
    int num;
    void *infile;
    uint8_t buffer[40];

    for (i = 0; i < argc; i++)
    {
        if (i == 0)
        {
            if (argc == 1 || stdinfile != stdin)
                infile = stdinfile;
            else
                continue;
        }
        else
        {
            infile = fopen(argv[i], "r");
            if (infile == 0)
            {
                perror(argv[i]);
                continue;
            }
        }
        if (infile == stdin)
        {
            while (gets(buffer))
            {
                if (buffer[0] == 4) break;
                fprintf(stdoutfile, "%s\n", buffer);
            }
        }
        else
        {
            while ((num = fread(buffer, 1, 40, infile)))
                fwrite(buffer, 1, num, stdoutfile);
        }
        if (i)
            fclose(infile);
    }
    fflush(stdout);
}

/* This routine deletes the files specified by the command line arguments */
void Remove(int argc, char **argv)
{
    int i;

    for (i = 1; i < argc; i++)
    {
        if (remove(argv[i]))
            perror(argv[i]);
    }
}

/* This routine echos the command line arguments */
void Echo(int argc, char **argv)
{
    int i;
    for (i = 1; i < argc; i++)
    {
        if (i != argc - 1)
            fprintf(stdoutfile, "%s ", argv[i]);
        else
            fprintf(stdoutfile, "%s\n", argv[i]);
    }
}

/* This routine lists the root directory or any subdirectories specified
   in the command line arguments.  If the "-l" option is specified, it
   will print the file attributes and size.  Otherwise, it will just
   print the file names.  */
void List(int argc, char **argv)
{
    int i, j;
    char *ptr;
    char fname[13];
    int32_t count = 0;
    uint32_t filesize;
    uint32_t longflag = 0;
    char *path;
    char drwx[5];
    int column;
    int prevlen;
    DIR *dirp;
    struct dirent *entry;

    // Check flags
    for (j = 1; j < argc; j++)
    {
        if (argv[j][0] == '-')
        {
            if (!strcmp(argv[j], "-l"))
                longflag = 1;
            else
                printf("Unknown option \"%s\"\n", argv[j]);
        }
        else
            count++;
    }

    // List directories
    for (j = 1; j < argc || count == 0; j++)
    {
        if (count == 0)
        {
            count--;
            path = "./";
        }
        else if (argv[j][0] == '-')
            continue;
        else
            path = argv[j];

        if (count >= 2)
            fprintf(stdoutfile, "\n%s:\n", path);

        dirp = opendir(path);

        if (!dirp)
        {
            perror(path);
            continue;
        }

        column = 0;
        prevlen = 14;
        while (entry = readdir(dirp))
        {
            if (entry->name[0] == '.') continue;
            ptr = fname;
            for (i = 0; i < 8; i++)
            {
                if (entry->name[i] == ' ') break;
                *ptr++ = tolower(entry->name[i]);
            }
            if (entry->name[8] != ' ')
            {
                *ptr++ = '.';
                for (i = 8; i < 11; i++)
                {
                    if (entry->name[i] == ' ') break;
                    *ptr++ = tolower(entry->name[i]);
                }
            }
            *ptr = 0;
            filesize = entry->filesize_3;
            filesize = (filesize << 8) | entry->filesize_2;
            filesize = (filesize << 8) | entry->filesize_1;
            filesize = (filesize << 8) | entry->filesize_0;
            strcpy(drwx, "-rw-");
            if (entry->attr & ATTR_READ_ONLY)
                drwx[2] = '-';
            if (entry->attr & ATTR_ARCHIVE)
                drwx[3] = 'x';
            if (entry->attr & ATTR_DIRECTORY)
            {
                drwx[0] = 'd';
                drwx[3] = 'x';
            }
            if (longflag)
                fprintf(stdoutfile, "%s %8d %s\n", drwx, filesize, fname);
            else if (++column == 5)
            {
                for (i = prevlen; i < 14; i++) fprintf(stdoutfile, " ");
                fprintf(stdoutfile, "%s\n", fname);
                column = 0;
                prevlen = 14;
            }
            else
            {
                for (i = prevlen; i < 14; i++) fprintf(stdoutfile, " ");
                prevlen = strlen(fname);
                fprintf(stdoutfile, "%s", fname);
            }
        }
        closedir(dirp);
        if (!longflag && column)
            fprintf(stdoutfile, "\n");
    }
}

/* This routine returns a pointer to the first character that doesn't
   match val. */
char *SkipChar(char *ptr, int val)
{
    while (*ptr)
    {
        if (*ptr != val) break;
        ptr++;
    }
    return ptr;
}

/* This routine returns a pointer to the first character that matches val. */
char *FindChar(char *ptr, int val)
{
    while (*ptr)
    {
        if (*ptr == val) break;
        ptr++;
    }
    return ptr;
}

/* This routine extracts tokens from a string that are separated by one or
   more spaces.  It returns the number of tokens found. */
int tokenize(char *ptr, char *tokens[])
{
    int num = 0;

    while (*ptr)
    {
        ptr = SkipChar(ptr, ' ');
        if (*ptr == 0) break;
        if (ptr[0] == '>')
        {
            ptr++;
            if (ptr[0] == '>')
            {
                tokens[num++] = ">>";
                ptr++;
            }
            else
                tokens[num++] = ">";
            continue;
        }
        if (ptr[0] == '<')
        {
            ptr++;
            tokens[num++] = "<";
            continue;
        }
        tokens[num++] = ptr;
        ptr = FindChar(ptr, ' ');
        if (*ptr) *ptr++ = 0;
    }
    return num;
}

/* This routine searches the list of tokens for the redirection operators
   and opens the files for input, output or append depending on the 
   operator. */
int CheckRedirection(char **tokens, int num)
{
    int i, j;

    for (i = 0; i < num-1; i++)
    {
        if (!strcmp(tokens[i], ">"))
        {
            stdoutfile = fopen(tokens[i+1], "w");
            if (!stdoutfile)
            {
                perror(tokens[i+1]);
                stdoutfile = stdout;
                return 0;
            }
        }
        else if (!strcmp(tokens[i], ">>"))
        {
            stdoutfile = fopen(tokens[i+1], "a");
            if (!stdoutfile)
            {
                perror(tokens[i+1]);
                stdoutfile = stdout;
                return 0;
            }
        }
        else if (!strcmp(tokens[i], "<"))
        {
            stdinfile = fopen(tokens[i+1], "r");
            if (!stdinfile)
            {
                perror(tokens[i+1]);
                stdinfile = stdin;
                return 0;
            }
        }
        else
            continue;
        for (j = i + 2; j < num; j++) tokens[j-2] = tokens[j];
        i--;
        num -= 2;
    }
    return num;
}

/* This routine closes files that were open for redirection */
void CloseRedirection()
{
    if (stdinfile != stdin)
    {
        fclose(stdinfile);
        stdinfile = stdin;
    }
    if (stdoutfile != stdout)
    {
        fclose(stdoutfile);
        stdoutfile = stdout;
    }
}

void mount()
{
    printf("Load and mount SD: ");
    _SD_Params* mountParams = (_SD_Params*)-1;

// Important: This code assumes you're using a C3 card.
// If you're using different hardware, make sure you
// change the following initialization to match your card!

#ifdef SPINNERET_CARD
    static _SD_Params params =
    {
        AttachmentType: _SDA_SingleSPI,
        pins:
        {
            SingleSPI:
            {
                MISO: 16,   // The pin attached to the SD card's MISO or DO output
                CLK:  21,   // The pin attached to the SD card's CLK or SCLK input
                MOSI: 20,   // The pin attached to the SD card's MOSI or DI input
                CS:   19    // The pin attached to the SD card's CS input
            }
        }
    };
    mountParams = &params;
#endif

#ifdef PROP_BOE /* Board of Education */
    static _SD_Params params =
    {
        AttachmentType: _SDA_SingleSPI,
        pins:
        {
            SingleSPI:
            {
                MISO: 22,   // The pin attached to the SD card's MISO or DO output
                CLK:  23,   // The pin attached to the SD card's CLK or SCLK input
                MOSI: 24,   // The pin attached to the SD card's MOSI or DI input
                CS:   25    // The pin attached to the SD card's CS input
            }
        }
    };
    mountParams = &params;
#endif

#define C3_CARD
#ifdef C3_CARD
    static _SD_Params params =
    {
        AttachmentType: _SDA_SerialDeMUX,
        pins:
        {
            SerialDeMUX:
            {
                MISO: 10,    // The pin attached to the SD card's MISO or DO output
                CLK:  11,    // The pin attached to the SD card's CLK or SCLK input
                MOSI: 9,     // The pin attached to the SD card's MOSI or DI input
                CLR:  25,    // The pin attached to the counter's reset/clear pin
                INC:  8,     // The pin attached to the counter's clock/count pin
                ADDR: 5,     // The SD card's demux address (the counter's count)
            }
        }
    };
    mountParams = &params;
#endif

#ifdef PARALLEL_SPI /* This is a hypothetical example - modify to suit your needs */
    static _SD_Params params =
    {
        AttachmentType: _SDA_ParallelDeMUX,
        pins:
        {
            ParallelDeMUX:
            {
                MISO: 4,    // The pin attached to the SD card's MISO or DO output
                CLK:  5,    // The pin attached to the SD card's CLK or SCLK input
                MOSI: 6,    // The pin attached to the SD card's MOSI or DI input
                CS:   0,    // The pin attached to the counter's reset/clear pin
                START: 2,   // The start bit of the pin mask to set when selecting the SD card's deMUX address
                WIDTH: 3,   // The width of the pin mask for all pins attached to the deMUX address
		ADDR: 5     // The value to write to the select field when selecting the SD card's deMUX address
            }
        }
    };
    mountParams = &params;
#endif

#if defined(__PROPELLER_XMMC__) && defined(SD_IS_USING_SD_CACHE_DRIVER)
    // Pass NULL as the params. In this case, we'll use the SD Cache driver.
    // Beware: This only works if you're running your program
    // cached off of the SD card (i.e. propeller-load -z).
    mountParams = 0;
#endif

    if (mountParams == (_SD_Params*)-1)
    {
        printf("You must specify the SD paramters in the filetest.c\n");
        exit(1);
    }

    uint32_t mountErr = dfs_mount(mountParams);
    if (mountErr)
    {
        printf("Mount error: %d\n", mountErr);
        exit(1);
    }

    printf("done.\n\n");
}

/* The program starts the file system.  It then loops reading commands
   and calling the appropriate routine to process it. */
int main()
{
    int num;
    char *tokens[20];
    uint8_t buffer[80];

    stdinfile = stdin;
    stdoutfile = stdout;

    // Wait for the serial terminal to start
    waitcnt(CNT + CLKFREQ);

    mount();

    Help();

    while (1)
    {
        printf("\n> ");
        fflush(stdout);
        gets(buffer);
        num = tokenize(buffer, tokens);
        num = CheckRedirection(tokens, num);
        if (num == 0) continue;
        if (!strcmp(tokens[0], "help"))
            Help();
        else if (!strcmp(tokens[0], "cat"))
            Cat(num, tokens);
        else if (!strcmp(tokens[0], "ls"))
            List(num, tokens);
        else if (!strcmp(tokens[0], "ll"))
        {
            tokens[num++] = "-l";
            List(num, tokens);
        }
        else if (!strcmp(tokens[0], "rm"))
            Remove(num, tokens);
        else if (!strcmp(tokens[0], "echo"))
            Echo(num, tokens);
        else if (!strcmp(tokens[0], "cd"))
            Cd(num, tokens);
        else if (!strcmp(tokens[0], "pwd"))
            Pwd(num, tokens);
        else if (!strcmp(tokens[0], "mkdir"))
            Mkdir(num, tokens);
        else if (!strcmp(tokens[0], "rmdir"))
            Rmdir(num, tokens);
        else
        {
            printf("Invalid command\n");
            Help();
        }
        CloseRedirection();
    }

    return 0;
}

/*
+--------------------------------------------------------------------
|  TERMS OF USE: MIT License
+--------------------------------------------------------------------
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
+------------------------------------------------------------------
*/
