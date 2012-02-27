/*******************************************************************************
' Author: Dave Hein
' Version 0.21
' Copyright (c) 2010, 2011
' See end of file for terms of use.
'******************************************************************************/
//#include <sys/types.h>
#include <stdint.h>

#define BYTE(addr) (((uint8_t *)hubram)[MAP_ADDR(addr)])
#define WORD(addr) (((uint16_t *)hubram)[MAP_ADDR(addr) >> 1])
#define LONG(addr) (((int32_t *)hubram)[MAP_ADDR(addr) >> 2])

// Define system I/O addresses and commands
#define SYS_COMMAND    0x12340000
#define SYS_LOCKNUM    0x12340002
#define SYS_PARM       0x12340004
#define SYS_DEBUG      0x12340008

// This struct is used by the PASM simulator
typedef struct PasmVarsS {
    int32_t mem[512];
    int32_t state;
    int32_t pc;
    int32_t cflag;
    int32_t zflag;
    int32_t cogid;
    int32_t waitflag;
    // P2 variables
    int32_t instruct1;
    int32_t instruct2;
    int32_t instruct3;
    int32_t pc1;
    int32_t pc2;
    int32_t pc3;
    int32_t cachehubaddr;
    int32_t cachecogaddr;
    int32_t ptra;
    int32_t ptrb;
    int32_t spa;
    int32_t spb;
    int32_t accl;
    int32_t acch;
    int32_t inda;
    int32_t indabot;
    int32_t indatop;
    int32_t indb;
    int32_t indbbot;
    int32_t indbtop;
    int32_t repcnt;
    int32_t repbot;
    int32_t reptop;
    int32_t cache[4];
    int32_t clut[128];
} PasmVarsT;

// This struct is used by the Spin simulator
// It is a subset of PasmVarsT starting at mem[0x1e0]
typedef struct SpinVarsS {
    int32_t x1e0;     // $1e0
    int32_t x1e1;     // $1e1
    int32_t x1e2;     // $1e2
    int32_t x1e3;     // $1e3
    int32_t x1e4;     // $1e4
    int32_t masklong; // $1e5
    int32_t masktop;  // $1e6
    int32_t maskwr;   // $1e7
    int32_t lsb;      // $1e8
    int32_t id;       // $1e9
    int32_t dcall;    // $1ea
    int32_t pbase;    // $1eb
    int32_t vbase;    // $1ec
    int32_t dbase;    // $1ed
    int32_t pcurr;    // $1ee
    int32_t dcurr;    // $1ef
    int32_t par;      // $1f0
    int32_t cnt;      // $1f1
    int32_t ina;      // $1f2
    int32_t inb;      // $1f3
    int32_t outa;     // $1f4
    int32_t outb;     // $1f5
    int32_t dira;     // $1f6
    int32_t dirb;     // $1f7
    int32_t ctra;     // $1f8
    int32_t ctrb;     // $1f9
    int32_t frqa;     // $1fa
    int32_t frqb;     // $1fb
    int32_t phsa;     // $1fc
    int32_t phsb;     // $1fd
    int32_t vcfg;     // $1fe
    int32_t vscl;     // $1ff
    int32_t state;
} SpinVarsT;

void RebootProp(void);
int32_t GetCnt(void);
void UpdatePins(void);
int32_t MAP_ADDR(int32_t addr);
void DebugPasmInstruction(PasmVarsT *pasmvars);
int  ExecutePasmInstruction(PasmVarsT *pasmvars);
void DebugPasmInstruction2(PasmVarsT *pasmvars);
int  ExecutePasmInstruction2(PasmVarsT *pasmvars);
void StartCog(SpinVarsT *spinvars, int par, int cogid);
void StartPasmCog(PasmVarsT *pasmvars, int par, int addr, int cogid);
void StartPasmCog2(PasmVarsT *pasmvars, int par, int addr, int cogid);
/*
+ -----------------------------------------------------------------------------------------------------------------------------+
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
