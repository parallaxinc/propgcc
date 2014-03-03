/*******************************************************************************
' Author: Dave Hein
' Version 0.54
' Copyright (c) 2012
' See end of file for terms of use.
'******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#ifdef LINUX
#include <dirent.h>
#include <sys/stat.h>
#include "conion.h"
#else
#include <conio.h>
#include <direct.h>
#endif
#include "interp.h"
#include "spindebug.h"
#include "eeprom.h"
#include "spinsim.h"

extern int32_t loopcount;
extern int32_t printflag;
extern int32_t baudrate;
extern int32_t eeprom;
extern char *hubram;

void GetDebugString(char *ptr);
int32_t RunProp(int32_t maxloops);

void Help(void)
{
    printf("Debug Commands\n");
    printf("help     - Print command list\n");
    printf("exit     - Exit spinsim\n");
    printf("step     - Run a single step of the simulator\n");
    printf("run      - Run continuously\n");
    printf("list on  - Turn on debug prints\n");
    printf("list off - Turn off debug prints\n");
}

void Debug(void)
{
    int runflag = 1;
    char buffer[200];
    int maxloops;

    while (runflag)
    {
        while (1)
        {
            printf("\nDEBUG> ");
            GetDebugString(buffer);
            if (!strcmp(buffer, "exit"))
            {
                runflag = 0;
                break;
            }
            else if (!strcmp(buffer, "step"))
            {
                maxloops = loopcount + 1;
                break;
            }
            else if (!strcmp(buffer, "list on"))
            {
                printflag = 0xffff;
                LONG(SYS_DEBUG) = printflag;
            }
            else if (!strcmp(buffer, "list off"))
            {
                printflag = 0;
                LONG(SYS_DEBUG) = printflag;
            }
            else if (!strcmp(buffer, "help"))
            {
                Help();
            }
            else if (!strcmp(buffer, "run"))
            {
                maxloops = -1;
                break;
            }
        }
        if (runflag) RunProp(maxloops);
    }
}

void GetDebugString(char *ptr)
{
    int value;

    while (1)
    {
        while (!kbhit());
        value = getch();
        putchx(value);
        if (value == 13 || value == 10)
        {
            *ptr = 0;
            return;
        }
        *ptr++ = value;
    }
}

int32_t RunProp(int32_t maxloops)
{
    int32_t runflag = 1;

    while (runflag && (maxloops < 0 || loopcount < maxloops))
    {
        runflag = step_chip();
        CheckCommand();
        if (baudrate)
        {
            CheckSerialOut();
            if (CheckSerialIn()) return 1;
        }
        if (eeprom)
            CheckEEProm();
    }
    return 0;
}
/*
+------------------------------------------------------------------------------------------------------------------------------+
|                                                   TERMS OF USE: MIT License                                                  |
+------------------------------------------------------------------------------------------------------------------------------+
|Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation    |
|files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,    |
|modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software|
|is furnished to do so, subject to the following conditions:                                                                   |
|                                                                                                                              |
|The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.|
|                                                                                                                              |
|THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE          |
|WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR         |
|COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,   |
|ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                         |
+------------------------------------------------------------------------------------------------------------------------------+
*/
