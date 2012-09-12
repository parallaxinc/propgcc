/**
 *
 * @file remove.c
 *
 * This is an rm-like program for use in Propeller-GCC windows MinGW distributions.
 * Makefiles tend to have a "clean" entry that uses rm.
 * Traditionally rm has -f to force remove even if file doesn't exist.
 * Traditionally rm has -r to recursively remove a directory.
 * rm can be used with -rf to recursively and forece remove a directory/files.
 *
 * DOS/Windows does not have one command to do all these functions.
 *
 * Syntax: rm [options] filename(s).
 * MinGW libraries will expand wild-card parameters as filenames.
 *
 */

/*
 * Copyright (c) 2011 by Parallax, Inc.
 * Coded by Steve Denson
 * MIT Licensed - see end of file.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#ifdef DEBUG
#define dprintf printf
#else
#define dprintf(...)
#endif

/* forward declarations */
void usage(void);
void doRM(char* filename, int options);
int  rmDirectory(char* filename, int options);
void verboseRemove(char* filename, int options);

#define FORCEBIT   1
#define RECURSEBIT 2
#define VERBOSEBIT 4

/* mingw path separator is '/' */
#define PATHSEP '/'

char *program;

/*
 * parse parameters and remove expanded file names
 */
int main(int argc, char* argv[])
{
    int n;              
    int options = 0;        // must be zero, used for detecting options
    char *filename = 0;
    
    /* use progname global for usage */
    program = argv[0];

    /* if we don't have enough arguments, flag an error */
    if(argc < 2)
    {
        /* usage never returns ... by request */
        usage();
    }

    /* find out what options to use */
    for(n = 1; n < argc; n++)
    {
        dprintf("argv[%d]: %s\n", n, argv[n]);
        if(argv[n][0] == '-')
        {
            int arglen = strlen(argv[n])-1;
            for(; arglen>0; arglen--)
            {
                switch (argv[n][arglen])
                {
                    case 'f':
                        options |= FORCEBIT;
                        break;
                    case 'r':   // allow r or R for recursive mode
                    case 'R':
                        options |= RECURSEBIT;
                        break;
                    case 'v':
                        options |= VERBOSEBIT;
                        break;
                    default:
                        printf("Invalid option %c\n",argv[n][arglen]);
                        usage();
                        break;
                }
            }
        }
        /* no '-', it's a filename */
        else
        {
            break;
        }
    }

    /* n is set by options parser code */
    for(; n < argc; n++)
    {
        filename = argv[n];

        dprintf("rm file '%s' options f %d r %d v %d\n", filename,
            options & FORCEBIT, options & RECURSEBIT, options & VERBOSEBIT);

        /* This should never happen .... don't process an empty file name. */
        if(*filename == 0 || filename == 0)
        {
            usage();
        }
        else
        {
            doRM(filename, options);
        }
    }

    /*
     * if we manage to get here instead of calling usage(),
     * it means the program should have worked.
     */
    return 0;
}

/*
 * print program usage
 * never returns - implemented like this by request
 */
void usage(void)
{
    printf("Usage: %s [-frRv] <filename>\n", program);
    printf("Remove file/directory from Windows drive. Options:\n");
    printf("-f Force removal of file. Ignore errors if file doesn't exist.\n");
    printf("-r [-R] Recursively remove directory and files.\n");
    printf("-v Report files being removed.\n");
    printf("<filename> exact spelling or wildcards like file*, file.*, or *.ext .\n");

    /* I personally dislike this method, but others do, so I'll use it. */
    exit(1);
}

/*
 * Do top level remove
 * handle all errors here where possible
 */
void doRM(char* filename, int options)
{
    int     statret = 0;
    struct  stat status;
    int     fnlen = strlen(filename);       // file name length

    /* file or directory must not end with PATHSEP */
    if(filename[fnlen-1] == PATHSEP)
    {
        filename[fnlen-1] = '\0';
    }

    /* get file status. if statret != 0, the file is invalid */
    statret = stat(filename, &status);
    if(statret != 0)
    {
        if(!(options & FORCEBIT))
        {
            perror(filename);
            if(errno != ENOENT)
                usage();
            exit(errno); // just exit here with the error ... 
        }
        exit(0); // no error ... multiple exit points :(
    }

    /*
     * Do we have options ?
     * Without options, we assume a simple file.
     * Throw an error if the file is a directory and -r is not specified.
     */
    if(!options)
    {
        if(S_ISDIR(status.st_mode))
        {
            printf("%s is a directory.\n", filename);
            printf("Can't remove a directory without -r.\n");
            usage();
        }
        else
        {
            /* verboseRemove() doesn't check FORCEBIT */
            verboseRemove(filename, options);
        }
    }

    /* We have options. handle the cases */
    else
    {
        if(S_ISDIR(status.st_mode))
        {
            if(!(options & RECURSEBIT))
            {
                printf("%s is a directory.\n", filename);
                printf("Can't remove a directory without -r.\n");
                usage();
            }
            else
            {
                if(rmDirectory(filename,options))
                    usage();
            }
        }
        else
        {
            verboseRemove(filename, options);
        }
    }
}

/*
 * is the file a directory?
 * returns non-zero if true.
 * if stat error, perror and terminate with usage().
 */
int isDirectory(char *filename)
{
    struct stat status;
    int rc = stat(filename, &status);

    /*
     * stat returns non-zero if there is an error.
     */
    if(rc)
    {
        perror(filename);
        usage();
    }
    else
    {
        return S_ISDIR(status.st_mode);
    }
    return 0;
}

/*
 * Recursively remove directory
 * Since it is recursive and uses malloc/free,
 * it will not exit on error by calling usage.
 * Of course most O/S clean up after apps, so that doesn't matter much.
 * Return non zero on error.
 */
int rmDirectory(char *filename, int options)
{
    int     retval  = 0;
    int     count   = 0;
    int     dnlen   = 0;
    DIR     *dirp   = 0;
    char    *myname = 0;
    struct  dirent *entry = 0;
    int     fnlen   = strlen(filename)+1;

    /* open the directory to read */
    dirp = opendir(filename);
    if(dirp)
    {
        while((entry = readdir(dirp)) != 0)
        {
            /* don't delete parent. delete this directory later. */
            if(++count<=2)
                continue;

            /* fnlen + dnlen should have enough space for strings and null */
            dnlen   = strlen(entry->d_name)+1;
            /* get space for full path/filename */
            myname = (char*) malloc(fnlen+dnlen);

            /* set path/filename */
            sprintf(myname,"%s%c%s",filename,PATHSEP,entry->d_name);
            if(isDirectory(myname))
            {
                retval += rmDirectory(myname, options);
            }
            else
            {
                verboseRemove(myname, options);
            }
            /* free path/filename for next entry */
            free(myname);
        }

        /* close the directory */
        closedir (dirp);
        
        /* delete the directory */
        if (rmdir(filename))
        {
            /* rmdir returns non-zero on error */
            retval++;
        }

        if(options & VERBOSEBIT)
        {
            printf("removed directory '%s'\n", filename);
        }
    }
    return retval;
}

/*
 * Interactive mode is disabled for now, but keep the code for later.
 * This function does not check for FORCEBIT.
 */
void verboseRemove(char *filename, int options)
{
    remove(filename);
    if(options & VERBOSEBIT)
    {
        printf("removed '%s'\n", filename);
    }
}

/*
+--------------------------------------------------------------------
TERMS OF USE: MIT License
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
+--------------------------------------------------------------------
*/
