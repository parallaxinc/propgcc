/*******************************************************************************
' Author: Dave Hein
' Version 0.54
' Copyright (c) 2012
' See end of file for terms of use.
'******************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

extern int32_t pin_val;
extern int32_t eeprom;

static int32_t scl_prev = 1;
static int32_t sda_prev = 1;
static int32_t state = 0;
static int32_t count = 0;
static int32_t value = 0;
static int32_t control = 0;
static int32_t address = 0;
static int32_t drivepin = 0;
static int32_t driveval = 0;

static unsigned char memory[256*256];

void CheckEEProm()
{
    int32_t scl = (pin_val >> 28) & 1;
    int32_t sda = (pin_val >> 29) & 1;

    if (!eeprom) return;

    //printf("scl = %d, sda = %d\n", scl, sda);

    if (!drivepin && scl_prev && sda_prev && scl && !sda)
    {
        //printf("Start\n");
        count = 0;
        state = 1;
        control = 0;
        drivepin = 0;
    }
    else if (!drivepin && scl_prev && !sda_prev && scl && sda)
    {
        //printf("Stop\n");
        state = 0;
        drivepin = 0;
    }
    else if (state && scl_prev && !scl)
    {
        drivepin = 0;
        if (state == 1)
        {
            if (count) control = (control << 1) | sda_prev;
            count++;
            if (count == 9)
            {
                //printf("control = %2.2x\n", control);
                if ((control & 0xf0) == 0xa0)
                {
                    drivepin = 1;
                    driveval = 0;
                    count = 0;
                    if (control & 1)
                        state = 5;
                    else
                        state = 2;
                }
                else
                {
                    state = 0;
                }
            }
        }
        else if (state == 2 || state == 3)
        {
            if (count) address = (address << 1) | sda_prev;
            else if (state == 2) address = 0;
            count++;
            if (count == 9)
            {
                //if (state == 3) printf("address = %2.2x\n", address);
                state++;
                drivepin = 1;
                driveval = 0;
                count = 0;
            }
        }
        else if (state == 4)
        {
            if (count) value = (value << 1) | sda_prev;
            count++;
            if (count == 9)
            {
                //printf("value = %2.2x\n", value);
                memory[address] = value;
                address = (address + 1) & 0xffff;
                drivepin = 1;
                driveval = 0;
                count = 0;
                eeprom = 2;
            }
        }
        else if (state == 5)
        {
            if (count == 0)
            {
                if (sda_prev)
                    state = 0;
                else
                {
                    value = memory[address];
                    drivepin = 1;
                    driveval = (value >> 7) & 1;
                    value <<= 1;
                    count++;
                    address = (address + 1) & 0xffff;
                }
            }
            else if (count < 8)
            {
                drivepin = 1;
                driveval = (value >> 7) & 1;
                value <<= 1;
                count++;
            }
            else
            {
                count = 0;
                drivepin = 0;
            }
        }
    }
    if (drivepin)
    {
        pin_val = (pin_val & (~(1 << 29))) | (driveval << 29);
        sda = driveval;
    }
    scl_prev = scl;
    sda_prev = sda;
}

static FILE *OpenFile(char *fname, char *mode)
{
    FILE *file = fopen(fname, mode);
    if (!file)
    {
        printf("Could not open %s\n", fname);
        exit(1);
    }
    return file;
}

void EEPromInit(char *fname)
{
    FILE *file;

    if (!eeprom) return;

    //printf("EEPromInit(%s)\n", fname);

    if (fname)
    {
        file = fopen("eeprom.dat", "r");
        if (file)
        {
            fread(memory, 1, 65536, file);
            fclose(file);
            memset(memory, 0, 32768);
        }
        else
            memset(memory, 0, 65536);
        file = OpenFile(fname, "r");
        fread(memory, 1, 65536, file);
        fclose(file);
        file = OpenFile("eeprom.dat", "w");
        fwrite(memory, 1, 65536, file);
        fclose(file);
        return;
    }
    file = OpenFile("eeprom.dat", "r");
    fread(memory, 1, 65536, file);
    fclose(file);
}

void EEPromClose(void)
{
    FILE *file;

    if (eeprom == 2)
    {
        file = OpenFile("eeprom.dat", "w");
        fwrite(memory, 1, 65536, file);
        fclose(file);
    }
}

void EEPromCopy(char *mem)
{
    if (!eeprom) return;

    //printf("EEPromCopy\n");
    memcpy(mem, memory, 32768);
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
