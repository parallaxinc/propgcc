/* db_vmdebug.c - debug routines
 *
 * Copyright (c) 2009 by David Michael Betz.  All rights reserved.
 *
 */

#include <stdio.h>
#include <string.h>
#include "db_vmdebug.h"
#include "db_image.h"
#include "db_system.h"

/* instruction output formats */
#define FMT_NONE        0
#define FMT_BYTE        1
#define FMT_2BYTES      2
#define FMT_WORD        3
#define FMT_BR          4

typedef struct {
    int code;
    char *name;
    int fmt;
} OTDEF;

static FLASH_SPACE OTDEF otab[] = {
{ OP_HALT,      "HALT",     FMT_NONE    },
{ OP_BRT,       "BRT",      FMT_BR      },
{ OP_BRTSC,     "BRTSC",    FMT_BR      },
{ OP_BRF,       "BRF",      FMT_BR      },
{ OP_BRFSC,     "BRFSC",    FMT_BR      },
{ OP_BR,        "BR",       FMT_BR      },
{ OP_NOT,       "NOT",      FMT_NONE    },
{ OP_NEG,       "NEG",      FMT_NONE    },
{ OP_ADD,       "ADD",      FMT_NONE    },
{ OP_SUB,       "SUB",      FMT_NONE    },
{ OP_MUL,       "MUL",      FMT_NONE    },
{ OP_DIV,       "DIV",      FMT_NONE    },
{ OP_REM,       "REM",      FMT_NONE    },
{ OP_BNOT,      "BNOT",     FMT_NONE    },
{ OP_BAND,      "BAND",     FMT_NONE    },
{ OP_BOR,       "BOR",      FMT_NONE    },
{ OP_BXOR,      "BXOR",     FMT_NONE    },
{ OP_SHL,       "SHL",      FMT_NONE    },
{ OP_SHR,       "SHR",      FMT_NONE    },
{ OP_LT,        "LT",       FMT_NONE    },
{ OP_LE,        "LE",       FMT_NONE    },
{ OP_EQ,        "EQ",       FMT_NONE    },
{ OP_NE,        "NE",       FMT_NONE    },
{ OP_GE,        "GE",       FMT_NONE    },
{ OP_GT,        "GT",       FMT_NONE    },
{ OP_LIT,       "LIT",      FMT_WORD    },
{ OP_GREF,      "GREF",     FMT_WORD    },
{ OP_GSET,      "GSET",     FMT_WORD    },
{ OP_LREF,      "LREF",     FMT_BYTE    },
{ OP_LSET,      "LSET",     FMT_BYTE    },
{ OP_VREF,      "VREF",     FMT_NONE    },
{ OP_VSET,      "VSET",     FMT_NONE    },
{ OP_LITH,      "LITH",     FMT_WORD    },
{ OP_GREFH,     "GREFH",    FMT_WORD    },
{ OP_GSETH,     "GSETH",    FMT_WORD    },
{ OP_LREFH,     "LREFH",    FMT_BYTE    },
{ OP_LSETH,     "LSETH",    FMT_BYTE    },
{ OP_VREFH,     "VREFH",    FMT_NONE    },
{ OP_VSETH,     "VSETH",    FMT_NONE    },
{ OP_RESERVE,   "RESERVE",  FMT_2BYTES  },
{ OP_CALL,      "CALL",     FMT_NONE    },
{ OP_RETURN,    "RETURN",   FMT_2BYTES  },
{ OP_RETURNH,   "RETURNH",  FMT_2BYTES  },
{ OP_RETURNV,   "RETURNV",  FMT_2BYTES  },
{ OP_DROP,      "DROP",     FMT_NONE    },
{ OP_DROPH,     "DROPH",    FMT_NONE    },
{ OP_CAT,       "CAT",      FMT_NONE    },
{ 0,            NULL,       0           }
};

/* DecodeFunction - decode the instructions in a function code object */
void DecodeFunction(VMUVALUE base, const uint8_t *code, int len)
{
    const uint8_t *lc = code;
    const uint8_t *end = code + len;
    while (lc < end)
        lc += DecodeInstruction(base, code, lc);
}

/* DecodeInstruction - decode a single bytecode instruction */
int DecodeInstruction(VMUVALUE base, const uint8_t *code, const uint8_t *lc)
{
    uint8_t opcode, bytes[sizeof(VMVALUE)];
    const uint8_t *p;
    const OTDEF *op;
    VMVALUE offset;
    int n, addr, i;

    /* get the opcode */
    opcode = VMCODEBYTE(lc);

    /* show the address */
    addr = (int)(base + lc - code);
    VM_printf("%08x %04x %02x ", lc, addr, opcode);
    n = 1;

    /* display the operands */
    for (op = otab; op->name; ++op)
        if (opcode == op->code) {
            switch (op->fmt) {
            case FMT_NONE:
                for (i = 0; i < sizeof(VMVALUE); ++i)
                    VM_printf("   ");
                VM_printf("%s\n", op->name);
                break;
            case FMT_BYTE:
                bytes[0] = VMCODEBYTE(lc + 1);
                VM_printf("%02x ", bytes[0]);
                for (i = 1; i < sizeof(VMVALUE); ++i)
                    VM_printf("   ");
                VM_printf("%s %02x\n", op->name, bytes[0]);
                n += 1;
                break;
            case FMT_2BYTES:
                bytes[0] = VMCODEBYTE(lc + 1);
                bytes[1] = VMCODEBYTE(lc + 2);
                VM_printf("%02x %02x ", bytes[0], bytes[1]);
                for (i = 2; i < sizeof(VMVALUE); ++i)
                    VM_printf("   ");
                VM_printf("%s %02x %02x\n", op->name, bytes[0], bytes[1]);
                n += 2;
                break;
            case FMT_WORD:
                for (i = 0; i < sizeof(VMVALUE); ++i) {
                    bytes[i] = VMCODEBYTE(lc + i + 1);
                    VM_printf("%02x ", bytes[i]);
                }
                VM_printf("%s ", op->name);
                for (i = sizeof(VMVALUE); --i >= 0; )
                    VM_printf("%02x", bytes[i]);
                VM_printf("\n");
                n += sizeof(VMVALUE);
                break;
            case FMT_BR:
                p = lc + 1;
                get_VMVALUE(offset, VMCODEBYTE(p++));
                for (i = 0; i < sizeof(VMVALUE); ++i) {
                    bytes[i] = VMCODEBYTE(lc + i + 1);
                    VM_printf("%02x ", bytes[i]);
                }
                VM_printf("%s ", op->name);
                for (i = sizeof(VMVALUE); --i >= 0; )
                    VM_printf("%02x", bytes[i]);
                VM_printf(" # %04x\n", addr + 1 + sizeof(VMVALUE) + offset);
                n += sizeof(VMVALUE);
                break;
            }
            return n;
        }
            
    /* unknown opcode */
    VM_printf("      <UNKNOWN>\n");
    return 1;
}

