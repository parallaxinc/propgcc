/* Opcode table for Parallax Propeller
   Copyright 2011 Parallax Inc.

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; see the file COPYING.  If not, write to the
   Free Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "dis-asm.h"
#include "opcode/propeller.h"

/* To control whether wr or nr prints as blank, and
 * whether the r bit is set by default */
#define CC         FLAG_CC
#define CCZ        (FLAG_CC | FLAG_Z)
#define CCC        (FLAG_CC | FLAG_C)
#define CCZC       (FLAG_CC | FLAG_Z | FLAG_C)
#define CCZCWR     (FLAG_CC | FLAG_Z | FLAG_C | FLAG_R | FLAG_R_DEF)
#define CCZCNR     (FLAG_CC | FLAG_Z | FLAG_C | FLAG_R)
#define CCWR       (FLAG_CC | FLAG_R | FLAG_R_DEF)
#define CCCWR      (FLAG_CC | FLAG_C | FLAG_R | FLAG_R_DEF)

const struct propeller_opcode propeller_opcodes[] = {
/*
   mnemonic  insn  zcri cond    dst       src */
/* nop      ------ ---- cccc --------- --------- */
  {"nop", 0x00000000, 0xffffffff, PROPELLER_OPERAND_IGNORE, CCZCNR, PROP_1 | PROP_2, COMPRESS_MACRO, 0x00},

  /* we put the pseudo-instructions here so the disassembler gets a first
     crack at them
  */
  /* brs is a fake instruction that expands to either add or sub of a pc relative offset */
/* brs      100000 zcri cccc ddddddddd sssssssss */
  {"brs", 0x80000000, 0xfc000000, PROPELLER_OPERAND_BRS, CCZCWR, PROP_1_LMM | PROP_2_LMM, COMPRESS_BRL, 0},
  /* dummy entry for the disassembler only */
  {"brs ", 0x84800000, 0xfc800000, PROPELLER_OPERAND_BRS, CCZCWR, PROP_1_LMM | PROP_2_LMM, COMPRESS_BRL, 0},

/* ldi is a fake instruction built from a rdlong (rdlongc for p2) and a constant that decodes as NOP */
/* ldi      000010 zc1i cccc ddddddddd sssssssss */
  {"ldi", 0x08800000, 0xfc800000, PROPELLER_OPERAND_LDI, CCZCWR, PROP_1_LMM, NO_COMPRESSED, 0},
  {"ldi", 0x09800000, 0xfd800000, PROPELLER_OPERAND_LDI, CCZCWR, PROP_2_LMM, NO_COMPRESSED, 0},

/* brw is made of a jmp and a constant.  We may shrink it later. */
/* brl is the same; the only difference is how they expand in compressed mode
   (brw becomes a 16 bit relative branch, brl is a 32 bit absolute
*/
/* brw       000010 zc1i cccc ddddddddd sssssssss */
  {"brw", 0x5c000000, 0xffffffff, PROPELLER_OPERAND_BRW, CCZCNR, PROP_1_LMM, NO_COMPRESSED, 0},
  {"brw", 0x1c000000, 0xffffffff, PROPELLER_OPERAND_BRW, CCZCNR, PROP_2_LMM, NO_COMPRESSED, 0},
  {"brl", 0x5c000000, 0xffffffff, PROPELLER_OPERAND_BRL, CCZCNR, PROP_1_LMM, NO_COMPRESSED, 0},
  {"brl", 0x1c000000, 0xffffffff, PROPELLER_OPERAND_BRL, CCZCNR, PROP_2_LMM, NO_COMPRESSED, 0},

/* xmmio is made up of a mov immediate followed by a call; the first part
   is mov, so that's what we give here
*/
/* xmmio      101000 zcri cccc ddddddddd sssssssss */
  {"xmmio", 0xa0000000, 0xfc000000, PROPELLER_OPERAND_XMMIO, CCZCWR, PROP_1_LMM|PROP_2_LMM, NO_COMPRESSED, 0},

/* fcache is a jmp followed by a 32 bit constant */
  {"fcache", 0x5c000000, 0xffffffff, PROPELLER_OPERAND_FCACHE, CCZCNR, PROP_1_LMM, COMPRESS_MACRO, MACRO_FCACHE},
/* fcache is a jmp followed by a 32 bit constant */
  {"fcache", 0x1c000000, 0xffffffff, PROPELLER_OPERAND_FCACHE, CCZCNR, PROP_2_LMM, COMPRESS_MACRO, MACRO_FCACHE},
/* mvi expands to a jmp followed by a 32 bit constant, just like fcache */
  {"mvi", 0x5c000000, 0xffffffff, PROPELLER_OPERAND_MVI, CCZCNR, PROP_1_LMM, COMPRESS_MVI, PREFIX_MVI},
/* mvi expands to a jmp followed by a 32 bit constant, just like fcache */
  {"mvi", 0x1c000000, 0xffffffff, PROPELLER_OPERAND_MVI, CCZCNR, PROP_2_LMM, COMPRESS_MVI, PREFIX_MVI},
/* mviw expands to a jmp followed by a 32 bit constant, just like fcache */
  {"mviw", 0x5c000000, 0xffffffff, PROPELLER_OPERAND_MVI, CCZCNR, PROP_1_LMM, COMPRESS_MVIW, PREFIX_MVIW},
/* mviw expands to a jmp followed by a 32 bit constant, just like fcache */
  {"mviw", 0x1c000000, 0xffffffff, PROPELLER_OPERAND_MVI, CCZCNR, PROP_2_LMM, COMPRESS_MVIW, PREFIX_MVIW},
/* lcall expands to a jmp followed by a 32 bit constant, just like fcache */
  {"lcall", 0x5c000000, 0xffffffff, PROPELLER_OPERAND_LCALL, CCZCNR, PROP_1_LMM, COMPRESS_MACRO, MACRO_LCALL},
/* lcall expands to a jmp followed by a 32 bit constant, just like fcache */
  {"lcall", 0x1c000000, 0xffffffff, PROPELLER_OPERAND_LCALL, CCZCNR, PROP_2_LMM, COMPRESS_MACRO, MACRO_LCALL},

  /* lret expands to "mov pc, lr" */
  {"lret", 0xa0000000, 0xffffffff, PROPELLER_OPERAND_LRET, CCZCWR, PROP_1_LMM|PROP_2_LMM, COMPRESS_MACRO, MACRO_RET},

  /* other macros expand to a call */
  {"lmul", 0x5c800000, 0xffffffff, PROPELLER_OPERAND_MACRO_0, CCZCWR, PROP_1_LMM, COMPRESS_MACRO, MACRO_MUL},
  {"lmul", 0x1c800000, 0xffffffff, PROPELLER_OPERAND_MACRO_0, CCZCWR, PROP_2_LMM, COMPRESS_MACRO, MACRO_MUL},
  {"ludiv", 0x5c800000, 0xffffffff, PROPELLER_OPERAND_MACRO_0, CCZCWR, PROP_1_LMM, COMPRESS_MACRO, MACRO_UDIV},
  {"ludiv", 0x1c800000, 0xffffffff, PROPELLER_OPERAND_MACRO_0, CCZCWR, PROP_2_LMM, COMPRESS_MACRO, MACRO_UDIV},
  {"ldiv", 0x5c800000, 0xffffffff, PROPELLER_OPERAND_MACRO_0, CCZCWR, PROP_1_LMM, COMPRESS_MACRO, MACRO_DIV},
  {"ldiv", 0x1c800000, 0xffffffff, PROPELLER_OPERAND_MACRO_0, CCZCWR, PROP_2_LMM, COMPRESS_MACRO, MACRO_DIV},

/* pushm and popm expand to a mov followed by a jmpret */
/* push and pop      101000 zcri cccc ddddddddd sssssssss */
  {"lpushm", 0xa0000000, 0xffffffff, PROPELLER_OPERAND_MACRO_8, CCZCWR, PROP_1_LMM | PROP_2_LMM, COMPRESS_MACRO, MACRO_PUSHM},
  {"lpopm", 0xa0000000, 0xffffffff, PROPELLER_OPERAND_MACRO_8, CCZCWR, PROP_1_LMM | PROP_2_LMM, COMPRESS_MACRO, MACRO_POPM},
  {"lpopret", 0xa0000000, 0xffffffff, PROPELLER_OPERAND_MACRO_8, CCZCWR, PROP_1_LMM | PROP_2_LMM, COMPRESS_MACRO, MACRO_POPRET},

/* leasp expands to a mov rN,sp followed by add rN,#x */
/* it's mainly intended for compressed mode, but we have to support the
   expanded version for fcache */
/* leasp      101000 zcri cccc ddddddddd sssssssss */
  {"leasp", 0xa0000000, 0xffffffff, PROPELLER_OPERAND_LEASP, CCZCWR, PROP_1_LMM | PROP_2_LMM, COMPRESS_MACRO, PREFIX_LEASP},

/* xmov expands to a mov rA,rB followed by OP rC,rD */
/* it's mainly intended for compressed mode, but we have to support the
   expanded version for fcache */
/* leasp      101000 zcri cccc ddddddddd sssssssss */
  {"xmov", 0xa0800000, 0xffffffff, PROPELLER_OPERAND_XMOV, CCZCWR, PROP_1_LMM | PROP_2_LMM, COMPRESS_XMOV, PREFIX_XMOVREG},

/* wrbyte   000000 zc0i cccc ddddddddd sssssssss */
  {"wrbyte", 0x00000000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, CCZCNR, PROP_1, COMPRESS_XOP, XOP_WRB},
/* rdbyte   000000 zc1i cccc ddddddddd sssssssss */
  {"rdbyte", 0x00800000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1, COMPRESS_XOP, XOP_RDB},
/* wrword   000001 zc0i cccc ddddddddd sssssssss */
  {"wrword", 0x04000000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, CCZCNR, PROP_1, NO_COMPRESSED, 0},
/* rdword   000001 zc1i cccc ddddddddd sssssssss */
  {"rdword", 0x04800000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1, NO_COMPRESSED, 0},
/* wrlong   000010 zc0i cccc ddddddddd sssssssss */
  {"wrlong", 0x08000000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, CCZCNR, PROP_1, COMPRESS_XOP, XOP_WRL},
/* rdlong   000010 zc1i cccc ddddddddd sssssssss */
  {"rdlong", 0x08800000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1, COMPRESS_XOP, XOP_RDL},

/* clkset   000011 zc01 cccc ddddddddd 000000000 */
  {"clkset", 0x0c400000, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCNR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* cogid    000011 zcr1 cccc ddddddddd 000000001 */
  {"cogid", 0x0c400001, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* coginit  000011 zcR1 cccc ddddddddd 000000010 */
  {"coginit", 0x0c400002, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCNR, PROP_1, NO_COMPRESSED, 0},
/* cogstop  000011 zcR1 cccc ddddddddd 000000011 */
  {"cogstop", 0x0c400003, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCNR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* locknew  000011 zcr1 cccc ddddddddd 000000100 */
  {"locknew", 0x0c400004, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* lockret  000011 zcR1 cccc ddddddddd 000000101 */
  {"lockret", 0x0c400005, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCNR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* lockset  000011 zcR1 cccc ddddddddd 000000110 */
  {"lockset", 0x0c400006, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCNR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* lockclr  000011 zcR1 cccc ddddddddd 000000111 */
  {"lockclr", 0x0c400007, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCNR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* hubop    000011 zcRi cccc ddddddddd sssssssss */
  {"hubop", 0x0c000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCNR, PROP_1, NO_COMPRESSED, 0},

/* ror      001000 zcri cccc ddddddddd sssssssss */
  {"ror", 0x20000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* rol      001001 zcri cccc ddddddddd sssssssss */
  {"rol", 0x24000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* shr      001010 zcri cccc ddddddddd sssssssss */
  {"shr", 0x28000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, COMPRESS_XOP, XOP_SHR},
/* shl      001011 zcri cccc ddddddddd sssssssss */
  {"shl", 0x2c000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, COMPRESS_XOP, XOP_SHL},
/* rcr      001100 zcri cccc ddddddddd sssssssss */
  {"rcr", 0x30000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* rcl      001101 zcri cccc ddddddddd sssssssss */
  {"rcl", 0x34000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* sar      001110 zcri cccc ddddddddd sssssssss */
  {"sar", 0x38000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, COMPRESS_XOP, XOP_SAR},
/* rev      001111 zcri cccc ddddddddd sssssssss */
  {"rev", 0x3c000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* mins     010000 zcri cccc ddddddddd sssssssss */
  {"mins", 0x40000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* maxs     010001 zcri cccc ddddddddd sssssssss */
  {"maxs", 0x44000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* min      010010 zcri cccc ddddddddd sssssssss */
  {"min", 0x48000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* max      010011 zcri cccc ddddddddd sssssssss */
  {"max", 0x4c000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* movs     010100 zcri cccc ddddddddd sssssssss */
  {"movs", 0x50000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* movd     010101 zcri cccc ddddddddd sssssssss */
  {"movd", 0x54000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* movi     010110 zcri cccc ddddddddd sssssssss */
  {"movi", 0x58000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},

/* jmp      010111 zc0i cccc --------- sssssssss *//* These two are in the */
  {"jmp", 0x5c000000, 0xfc83fe00, PROPELLER_OPERAND_JMP, CCZCNR, PROP_1, NO_COMPRESSED, 0},
/* jmpn     010111 zc0i cccc --------- sssssssss */
  {"jmpn", 0x5c000000, 0xfc800000, PROPELLER_OPERAND_JMPRET, CCZCNR, PROP_1, NO_COMPRESSED, 0},
/* ret      010111 zc01 cccc --------- --------- *//* wrong order either way */
  {"ret", 0x5c400000, 0xfcc00000, PROPELLER_OPERAND_NO_OPS, CCZCNR, PROP_1, NO_COMPRESSED, 0},
/* jmpret   010111 zc1i cccc ddddddddd sssssssss *//* these, */
  {"jmpret", 0x5c800000, 0xfc800000, PROPELLER_OPERAND_JMPRET, CCZCWR, PROP_1, NO_COMPRESSED, 0},
/* call     010111 zc11 cccc DDDDDDDDD sssssssss *//* too. */
  {"call", 0x5c000000, 0xfc000000, PROPELLER_OPERAND_CALL, CCZCWR, PROP_1, NO_COMPRESSED, 0},

/* test     011000 zc0i cccc ddddddddd sssssssss */
  {"test", 0x60000000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, CCZCNR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* and      011000 zcri cccc ddddddddd sssssssss */
  {"and", 0x60000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, COMPRESS_XOP, XOP_AND},
/* testn     011001 zc0i cccc ddddddddd sssssssss */
  {"testn", 0x64000000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, CCZCNR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* andn     011001 zcri cccc ddddddddd sssssssss */
  {"andn", 0x64000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, COMPRESS_XOP, XOP_ANDN},
/* or       011010 zcri cccc ddddddddd sssssssss */
  {"or", 0x68000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, COMPRESS_XOP, XOP_OR},
/* xor      011011 zcri cccc ddddddddd sssssssss */
  {"xor", 0x6c000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, COMPRESS_XOP, XOP_XOR},
/* muxc     011100 zcri cccc ddddddddd sssssssss */
  {"muxc", 0x70000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* muxnc    011101 zcri cccc ddddddddd sssssssss */
  {"muxnc", 0x74000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* muxz     011110 zcri cccc ddddddddd sssssssss */
  {"muxz", 0x78000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* muxnz    011111 zcri cccc ddddddddd sssssssss */
  {"muxnz", 0x7c000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* add      100000 zcri cccc ddddddddd sssssssss */
  {"add", 0x80000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, COMPRESS_XOP, XOP_ADD},
/* cmp      100001 zc0i cccc ddddddddd sssssssss */
  {"cmp", 0x84000000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, CCZCNR, PROP_1 | PROP_2, COMPRESS_XOP, XOP_CMPU},
/* sub      100001 zc1i cccc ddddddddd sssssssss */
  {"sub", 0x84800000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, COMPRESS_XOP, XOP_SUB},
/* addabs   100010 zcri cccc ddddddddd sssssssss */
  {"addabs", 0x88000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* subabs   100011 zcri cccc ddddddddd sssssssss */
  {"subabs", 0x8c000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* sumc     100100 zcri cccc ddddddddd sssssssss */
  {"sumc", 0x90000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* sumnc    100101 zcri cccc ddddddddd sssssssss */
  {"sumnc", 0x94000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* sumz     100110 zcri cccc ddddddddd sssssssss */
  {"sumz", 0x98000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* sumnz    100111 zcri cccc ddddddddd sssssssss */
  {"sumnz", 0x9c000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* mov      101000 zcri cccc ddddddddd sssssssss */
  {"mov", 0xa0000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, COMPRESS_MOV, PREFIX_MVIB},
/* mova is like mov, but it assumes the immediate operand is a cog address */
/* mova      101000 zcri cccc ddddddddd sssssssss */
  {"mova", 0xa0000000, 0xfc000000, PROPELLER_OPERAND_MOVA, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* neg      101001 zcri cccc ddddddddd sssssssss */
  {"neg", 0xa4000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, COMPRESS_XOP, XOP_NEG},
/* abs      101010 zcri cccc ddddddddd sssssssss */
  {"abs", 0xa8000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* absneg   101011 zcri cccc ddddddddd sssssssss */
  {"absneg", 0xac000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* negc     101100 zcri cccc ddddddddd sssssssss */
  {"negc", 0xb0000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* negnc    101101 zcri cccc ddddddddd sssssssss */
  {"negnc", 0xb4000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* negz     101110 zcri cccc ddddddddd sssssssss */
  {"negz", 0xb8000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* negnz    101111 zcri cccc ddddddddd sssssssss */
  {"negnz", 0xbc000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* cmps     110000 zcRi cccc ddddddddd sssssssss */
  {"cmps", 0xc0000000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, CCZCNR, PROP_1 | PROP_2, COMPRESS_XOP, XOP_CMPS},
/* cmpsx    110001 zcRi cccc ddddddddd sssssssss */
  {"cmpsx", 0xc4000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCNR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* addx     110010 zcri cccc ddddddddd sssssssss */
  {"addx", 0xc8000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* cmpx     110011 zc0i cccc ddddddddd sssssssss */
  {"cmpx", 0xcc000000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, CCZCNR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* subx     110011 zc1i cccc ddddddddd sssssssss */
  {"subx", 0xcc800000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* adds     110100 zcri cccc ddddddddd sssssssss */
  {"adds", 0xd0000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* subs     110101 zcri cccc ddddddddd sssssssss */
  {"subs", 0xd4000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* addsx    110110 zcri cccc ddddddddd sssssssss */
  {"addsx", 0xd8000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* subsx    110111 zcri cccc ddddddddd sssssssss */
  {"subsx", 0xdc000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1 | PROP_2, NO_COMPRESSED, 0},
/* cmpsub   111000 zcri cccc ddddddddd sssssssss */
  {"cmpsub", 0xe0000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1, NO_COMPRESSED, 0},
/* djnz     111001 zcri cccc ddddddddd sssssssss */
  {"djnz", 0xe4000000, 0xfc000000, PROPELLER_OPERAND_JMPRET, CCZCWR, PROP_1, NO_COMPRESSED, 0},
/* tjnz     111010 zcRi cccc ddddddddd sssssssss */
  {"tjnz", 0xe8000000, 0xfc000000, PROPELLER_OPERAND_JMPRET, CCZCNR, PROP_1, NO_COMPRESSED, 0},
/* tjz      111011 zcRi cccc ddddddddd sssssssss */
  {"tjz", 0xec000000, 0xfc000000, PROPELLER_OPERAND_JMPRET, CCZCNR, PROP_1, NO_COMPRESSED, 0},
/* waitpeq  111100 zcRi cccc ddddddddd sssssssss */
  {"waitpeq", 0xf0000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCNR, PROP_1, NO_COMPRESSED, 0},
/* waitpne  111101 zcRi cccc ddddddddd sssssssss */
  {"waitpne", 0xf4000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCNR, PROP_1, NO_COMPRESSED, 0},
/* waitcnt  111110 zcri cccc ddddddddd sssssssss */
  {"waitcnt", 0xf8000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_1, NO_COMPRESSED, 0},
/* waitvid  111111 zcRi cccc ddddddddd sssssssss */
  {"waitvid", 0xfc000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCNR, PROP_1, NO_COMPRESSED, 0},

/* wrbyte   000000 000i cccc ddddddddd sssssssss */
  {"wrbyte", 0x00000000, 0xfc800000, PROPELLER_OPERAND_PTRS_OPS, CCZC, PROP_2, NO_COMPRESSED, 0},
/* rdbyte   000000 z01i cccc ddddddddd sssssssss */
  {"rdbyte", 0x00800000, 0xfd800000, PROPELLER_OPERAND_PTRS_OPS, CCZ, PROP_2, NO_COMPRESSED, 0},
/* rdbytec   000000 z11i cccc ddddddddd sssssssss */
  {"rdbytec", 0x01800000, 0xfd800000, PROPELLER_OPERAND_PTRS_OPS, CCZ, PROP_2, NO_COMPRESSED, 0},
/* wrword   000001 000i cccc ddddddddd sssssssss */
  {"wrword", 0x04000000, 0xfc800000, PROPELLER_OPERAND_PTRS_OPS, CCZC, PROP_2, NO_COMPRESSED, 0},
/* rdword   000001 001i cccc ddddddddd sssssssss */
  {"rdword", 0x04800000, 0xfd800000, PROPELLER_OPERAND_PTRS_OPS, CCZ, PROP_2, NO_COMPRESSED, 0},
/* rdwordc   000001 z11i cccc ddddddddd sssssssss */
  {"rdwordc", 0x05800000, 0xfd800000, PROPELLER_OPERAND_PTRS_OPS, CCZ, PROP_2, NO_COMPRESSED, 0},
/* wrlong   000010 000i cccc ddddddddd sssssssss */
  {"wrlong", 0x08000000, 0xfc800000, PROPELLER_OPERAND_PTRS_OPS, CCZC, PROP_2, NO_COMPRESSED, 0},
/* rdlong   000010 z01i cccc ddddddddd sssssssss */
  {"rdlong", 0x08800000, 0xfd800000, PROPELLER_OPERAND_PTRS_OPS, CCZ, PROP_2, NO_COMPRESSED, 0},
/* rdlongc   000010 z11i cccc ddddddddd sssssssss */
  {"rdlongc", 0x09800000, 0xfd800000, PROPELLER_OPERAND_PTRS_OPS, CCZ, PROP_2, NO_COMPRESSED, 0},

/* coginit  000011 zcr0 cccc ddddddddd sssssssss */
  {"coginit", 0x0c000000, 0xfc400000, PROPELLER_OPERAND_TWO_OPS, CCZCNR, PROP_2, NO_COMPRESSED, 0},

/* cachex  000011 zcr1 cccc 000000000 000001000 */
  {"cachex", 0x0c400008, 0xfc43ffff, PROPELLER_OPERAND_NO_OPS, CCZCNR, PROP_2, NO_COMPRESSED, 0},
/* clracca  000011 zcr1 cccc 000000001 000001000 */
  {"clracca", 0x0c400208, 0xfc43ffff, PROPELLER_OPERAND_NO_OPS, CCZCNR, PROP_2, NO_COMPRESSED, 0},
/* clraccb  000011 zcr1 cccc 000000010 000001000 */
  {"clraccb", 0x0c400408, 0xfc43ffff, PROPELLER_OPERAND_NO_OPS, CCZCNR, PROP_2, NO_COMPRESSED, 0},
/* clraccs  000011 zcr1 cccc 000000011 000001000 */
  {"clraccs", 0x0c400608, 0xfc43ffff, PROPELLER_OPERAND_NO_OPS, CCZCNR, PROP_2, NO_COMPRESSED, 0},
/* fitacca  000011 zcr1 cccc 000000101 000001000 */
  {"fitacca", 0x0c400a08, 0xfc43ffff, PROPELLER_OPERAND_NO_OPS, CCZCNR, PROP_2, NO_COMPRESSED, 0},
/* fitaccb  000011 zcr1 cccc 000000110 000001000 */
  {"fitaccb", 0x0c400c08, 0xfc43ffff, PROPELLER_OPERAND_NO_OPS, CCZCNR, PROP_2, NO_COMPRESSED, 0},
/* fitaccs  000011 zcr1 cccc 000000111 000001000 */
  {"fitaccs", 0x0c400e08, 0xfc43ffff, PROPELLER_OPERAND_NO_OPS, CCZCNR, PROP_2, NO_COMPRESSED, 0},
  
/* sndser  000011 zc01 cccc ddddddddd 000001001 */
  {"sndser", 0x0c400009, 0xfcc001ff, PROPELLER_OPERAND_DEST_ONLY, CCZC, PROP_2, NO_COMPRESSED, 0},
/* rcvser  000011 zc11 cccc ddddddddd 000001001 */
  {"rcvser", 0x0cc00009, 0xfcc001ff, PROPELLER_OPERAND_DEST_ONLY, CCZC, PROP_2, NO_COMPRESSED, 0},
  
/* pushzc  000011 zcr1 cccc ddddddddd 000001010 */
  {"pushzc", 0x0c40000a, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCNR, PROP_2, NO_COMPRESSED, 0},
/* popzc  000011 zcr1 cccc ddddddddd 000001011 */
  {"popzc", 0x0c40000b, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCNR, PROP_2, NO_COMPRESSED, 0},
  
/* subcnt  000011 zcr1 cccc ddddddddd 000001100 */
  {"subcnt", 0x0c40000c, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* passcnt  000011 zcr1 cccc ddddddddd 000001101 */
  {"passcnt", 0x0c40000d, 0xfcc001ff, PROPELLER_OPERAND_DEST_ONLY, CCZC, PROP_2, NO_COMPRESSED, 0},
/* getcnt    000011 zc11 cccc ddddddddd 000001101 */
  {"getcnt", 0x0cc0000d, 0xfcc001ff, PROPELLER_OPERAND_DEST_ONLY, CCZC, PROP_2, NO_COMPRESSED, 0},
/* getacca  000011 zcr1 cccc ddddddddd 000001110 */
  {"getacca", 0x0c40000e, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* getaccb  000011 zcr1 cccc ddddddddd 000001111 */
  {"getaccb", 0x0c40000f, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
  
/* getlfsr  000011 zcr1 cccc ddddddddd 000010000 */
  {"getlfsr", 0x0c400010, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* gettops  000011 zcr1 cccc ddddddddd 000010001 */
  {"gettops", 0x0c400011, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* getptra  000011 zcr1 cccc ddddddddd 000010010 */
  {"getptra", 0x0c400012, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* getptrb  000011 zcr1 cccc ddddddddd 000010011 */
  {"getptrb", 0x0c400013, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
  
/* getpix  000011 zcr1 cccc ddddddddd 000010100 */
  {"getpix", 0x0c400014, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* chkspd  000011 zcr1 cccc ddddddddd 000010101 */
  {"chkspd", 0x0c400015, 0xfcc001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCNR, PROP_2, NO_COMPRESSED, 0},
/* getspd  000011 zcr1 cccc ddddddddd 000010101 */
  {"getspd", 0x0c400015, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* chkspa  000011 zcr1 cccc ddddddddd 000010110 */
  {"chkspa", 0x0c400016, 0xfcc001ff, PROPELLER_OPERAND_NO_OPS, CCZCNR, PROP_2, NO_COMPRESSED, 0},
/* getspa  000011 zcr1 cccc ddddddddd 000010110 */
  {"getspa", 0x0c400016, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* chkspb  000011 zcr1 cccc ddddddddd 000010111 */
  {"chkspb", 0x0c400017, 0xfcc001ff, PROPELLER_OPERAND_NO_OPS, CCZCNR, PROP_2, NO_COMPRESSED, 0},
/* getspb  000011 zcr1 cccc ddddddddd 000010111 */
  {"getspb", 0x0c400017, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
  
/* popar  000011 zcr1 cccc ddddddddd 000011000 */
  {"popar", 0x0c400018, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* popbr  000011 zcr1 cccc ddddddddd 000011001 */
  {"popbr", 0x0c400019, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* popa  000011 zcr1 cccc ddddddddd 000011010 */
  {"popa", 0x0c40001a, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* popb  000011 zcr1 cccc ddddddddd 000011011 */
  {"popb", 0x0c40001b, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* reta  000011 zcr1 cccc 000000000 000011100 */
  {"reta", 0x0c40001c, 0xfc43ffff, PROPELLER_OPERAND_NO_OPS, CCZCNR, PROP_2, NO_COMPRESSED, 0},
/* retb  000011 zcr1 cccc 000000000 000011101 */
  {"retb", 0x0c40001d, 0xfc43ffff, PROPELLER_OPERAND_NO_OPS, CCZCNR, PROP_2, NO_COMPRESSED, 0},
/* retad  000011 zcr1 cccc 000000000 000011110 */
  {"retad", 0x0c40001e, 0xfc43ffff, PROPELLER_OPERAND_NO_OPS, CCZCNR, PROP_2, NO_COMPRESSED, 0},
/* retbd  000011 zcr1 cccc 000000000 000011111 */
  {"retbd", 0x0c40001f, 0xfc43ffff, PROPELLER_OPERAND_NO_OPS, CCZCNR, PROP_2, NO_COMPRESSED, 0},
  
/* decod2  000011 zcr1 cccc ddddddddd 000100000 */
  {"decod2", 0x0c400020, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* decod3  000011 zcr1 cccc ddddddddd 000100001 */
  {"decod3", 0x0c400021, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* decod4  000011 zcr1 cccc ddddddddd 000100010 */
  {"decod4", 0x0c400022, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* decod5  000011 zcr1 cccc ddddddddd 000100011 */
  {"decod5", 0x0c400023, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* blmask  000011 zcr1 cccc ddddddddd 000100100 */
  {"blmask", 0x0c400024, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* not  000011 zcr1 cccc ddddddddd 000100101 */
  {"not", 0x0c400025, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* onecnt  000011 zcr1 cccc ddddddddd 000100110 */
  {"onecnt", 0x0c400026, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* zercnt  000011 zcr1 cccc ddddddddd 000100111 */
  {"zercnt", 0x0c400027, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* incpat  000011 zcr1 cccc ddddddddd 000101000 */
  {"incpat", 0x0c400028, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* decpat  000011 zcr1 cccc ddddddddd 000101001 */
  {"decpat", 0x0c400029, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* bingry  000011 zcr1 cccc ddddddddd 000101010 */
  {"bingry", 0x0c40002a, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* grybin  000011 zcr1 cccc ddddddddd 000101011 */
  {"grybin", 0x0c40002b, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* mergew  000011 zcr1 cccc ddddddddd 000101100 */
  {"mergew", 0x0c40002c, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* splitw  000011 zcr1 cccc ddddddddd 000101101 */
  {"splitw", 0x0c40002d, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* seussf  000011 zcr1 cccc ddddddddd 000101110 */
  {"seussf", 0x0c40002e, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* seussr  000011 zcr1 cccc ddddddddd 000101111 */
  {"seussr", 0x0c40002f, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
  
/* getmull  000011 zcr1 cccc ddddddddd 000110000 */
  {"getmull", 0x0c400030, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* getmulh  000011 zcr1 cccc ddddddddd 000110001 */
  {"getmulh", 0x0c400031, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* getdivq  000011 zcr1 cccc ddddddddd 000110010 */
  {"getdivq", 0x0c400032, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* getdivr  000011 zcr1 cccc ddddddddd 000110011 */
  {"getdivr", 0x0c400033, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* getsqrt  000011 zcr1 cccc ddddddddd 000110100 */
  {"getsqrt", 0x0c400034, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* getqx  000011 zcr1 cccc ddddddddd 000110101 */
  {"getqx", 0x0c400035, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* getqy  000011 zcr1 cccc ddddddddd 000110110 */
  {"getqy", 0x0c400036, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* getqz  000011 zcr1 cccc ddddddddd 000110111 */
  {"getqz", 0x0c400037, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
  
/* getphsa  000011 zcr1 cccc ddddddddd 000111000 */
  {"getphsa", 0x0c400038, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* getphza  000011 zcr1 cccc ddddddddd 000111001 */
  {"getphza", 0x0c400039, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* getcos  000011 zcr1 cccc ddddddddd 000111010 */
  {"getcos", 0x0c40003a, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* getsin  000011 zcr1 cccc ddddddddd 000111011 */
  {"getsin", 0x0c40003b, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* getphsb  000011 zcr1 cccc ddddddddd 000111100 */
  {"getphsb", 0x0c40003c, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* getphzb  000011 zcr1 cccc ddddddddd 000111101 */
  {"getphzb", 0x0c40003d, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* getcosb  000011 zcr1 cccc ddddddddd 000111110 */
  {"getcosb", 0x0c40003e, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* getsinb  000011 zcr1 cccc ddddddddd 000111111 */
  {"getsinb", 0x0c40003f, 0xfc4001ff, PROPELLER_OPERAND_DEST_ONLY, CCZCWR, PROP_2, NO_COMPRESSED, 0},
  
/* repd  000011 z0n1 cccc nnnnnnnnn 001iiiiii */
  {"repd", 0x0c400040, 0xfd4001c0, PROPELLER_OPERAND_REPD, CCZ, PROP_2, NO_COMPRESSED, 0},
/* reps  000011 n111 nnnn nnnnnnnnn 001iiiiii */
  {"reps", 0x0dc00040, 0xfdc001c0, PROPELLER_OPERAND_REPS, 0, PROP_2, NO_COMPRESSED, 0},
  
///* jmptask  000011 zcn1 cccc nnnnnnnnn 01001tttt */
//  {"jmptask", 0x0c400090, 0xfc4001f0, PROPELLER_OPERAND_JMPTASK, CCZC, PROP_2, NO_COMPRESSED, 0},
/* jmptask  000011 zcn1 cccc nnnnnnnnn 01000tttt */
  {"jmptask", 0x0c400080, 0xfc4001f0, PROPELLER_OPERAND_JMPTASK, CCZC, PROP_2, NO_COMPRESSED, 0},
  
/* nopx  000011 zcn1 cccc nnnnnnnnn 010100000 */
  {"nopx", 0x0c4000a0, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setzc  000011 zcn1 cccc nnnnnnnnn 010100001 */
  {"setzc", 0x0c4000a1, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setspa  000011 zcn1 cccc nnnnnnnnn 010100010 */
  {"setspa", 0x0c4000a2, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setspb  000011 zcn1 cccc nnnnnnnnn 010100011 */
  {"setspb", 0x0c4000a3, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* addspa  000011 zcn1 cccc nnnnnnnnn 010100100 */
  {"addspa", 0x0c4000a4, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* addspb  000011 zcn1 cccc nnnnnnnnn 010100101 */
  {"addspb", 0x0c4000a5, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* subspa  000011 zcn1 cccc nnnnnnnnn 010100110 */
  {"subspa", 0x0c4000a6, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* subspb  000011 zcn1 cccc nnnnnnnnn 010100111 */
  {"subspb", 0x0c4000a7, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
  
/* pushar  000011 zcn1 cccc nnnnnnnnn 010101000 */
  {"pushar", 0x0c4000a8, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* pushbr  000011 zcn1 cccc nnnnnnnnn 010101001 */
  {"pushbr", 0x0c4000a9, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* pusha  000011 zcn1 cccc nnnnnnnnn 010101010 */
  {"pusha", 0x0c4000aa, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* pushb  000011 zcn1 cccc nnnnnnnnn 010101011 */
  {"pushb", 0x0c4000ab, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* calla  000011 zcn1 cccc nnnnnnnnn 010101100 */
  {"calla", 0x0c4000ac, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* callb  000011 zcn1 cccc nnnnnnnnn 010101101 */
  {"callb", 0x0c4000ad, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* callad  000011 zcn1 cccc nnnnnnnnn 010101110 */
  {"callad", 0x0c4000ae, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* callbd  000011 zcn1 cccc nnnnnnnnn 010101111 */
  {"callbd", 0x0c4000af, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
  
/* wrquad   000011 zcn1 cccc ddddddddd sssssssss */
  {"wrquad", 0x0c4000b0, 0xfc4001ff, PROPELLER_OPERAND_PTRD_OPS, CCZC, PROP_2, NO_COMPRESSED, 0},
/* rdquad   000011 z0i1 cccc ddddddddd sssssssss */
  {"rdquad", 0x0c4000b1, 0xfd4001ff, PROPELLER_OPERAND_PTRD_OPS, CCZ, PROP_2, NO_COMPRESSED, 0},
/* rdquadc   000011 z1i1 cccc ddddddddd sssssssss */
  {"rdquadc", 0x0d4000b1, 0xfd4001ff, PROPELLER_OPERAND_PTRD_OPS, CCZ, PROP_2, NO_COMPRESSED, 0},
/* setptra   000011 z0i1 cccc nnnnnnnnn 010110010 */
  {"setptra", 0x0c4000b2, 0xfd4001ff, PROPELLER_OPERAND_DESTIMM, CCZ, PROP_2, NO_COMPRESSED, 0},
/* setptrb   000011 zcn1 cccc nnnnnnnnn 010110011 */
  {"setptrb", 0x0c4000b3, 0xfd4001ff, PROPELLER_OPERAND_DESTIMM, CCZ, PROP_2, NO_COMPRESSED, 0},
/* addptra   000011 zcn1 cccc nnnnnnnnn 010110100 */
  {"addptra", 0x0c4000b4, 0xfd4001ff, PROPELLER_OPERAND_DESTIMM, CCZ, PROP_2, NO_COMPRESSED, 0},
/* addptrb   000011 zcn1 cccc nnnnnnnnn 010110101 */
  {"addptrb", 0x0c4000b5, 0xfd4001ff, PROPELLER_OPERAND_DESTIMM, CCZ, PROP_2, NO_COMPRESSED, 0},
/* subptra   000011 zcn1 cccc nnnnnnnnn 010110110 */
  {"subptra", 0x0c4000b6, 0xfd4001ff, PROPELLER_OPERAND_DESTIMM, CCZ, PROP_2, NO_COMPRESSED, 0},
/* subptrb   000011 zcn1 cccc nnnnnnnnn 010110111 */
  {"subptrb", 0x0c4000b7, 0xfd4001ff, PROPELLER_OPERAND_DESTIMM, CCZ, PROP_2, NO_COMPRESSED, 0},
  
/* setpix   000011 zcn1 cccc nnnnnnnnn 010111000 */
  {"setpix", 0x0c4000b8, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setpixu   000011 zcn1 cccc nnnnnnnnn 010111001 */
  {"setpixu", 0x0c4000b9, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setpixv   000011 zcn1 cccc nnnnnnnnn 010111010 */
  {"setpixv", 0x0c4000ba, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setpixz   000011 zcn1 cccc nnnnnnnnn 010111011 */
  {"setpixz", 0x0c4000bb, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setpixa   000011 zcn1 cccc nnnnnnnnn 010111100 */
  {"setpixa", 0x0c4000bc, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setpixr   000011 zcn1 cccc nnnnnnnnn 010111101 */
  {"setpixr", 0x0c4000bd, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setpixg   000011 zcn1 cccc nnnnnnnnn 010111110 */
  {"setpixg", 0x0c4000be, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setpixb   000011 zcn1 cccc nnnnnnnnn 010111111 */
  {"setpixb", 0x0c4000bf, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
  
/* setmulu   000011 z0n1 cccc nnnnnnnnn 011000000 */
  {"setmulu", 0x0c4000c0, 0xfd4001ff, PROPELLER_OPERAND_DESTIMM, CCZ, PROP_2, NO_COMPRESSED, 0},
/* setmula   000011 z1n1 cccc nnnnnnnnn 011000000 */
  {"setmula", 0x0d4000c0, 0xfd4001ff, PROPELLER_OPERAND_DESTIMM, CCZ, PROP_2, NO_COMPRESSED, 0},
/* setmulb   000011 zcn1 cccc nnnnnnnnn 011000001 */
  {"setmulb", 0x0c4000c1, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setdivu   000011 z0n1 cccc nnnnnnnnn 011000010 */
  {"setdivu", 0x0c4000c2, 0xfd4001ff, PROPELLER_OPERAND_DESTIMM, CCZ, PROP_2, NO_COMPRESSED, 0},
/* setdiva   000011 z1n1 cccc nnnnnnnnn 011000010 */
  {"setdiva", 0x0d4000c2, 0xfd4001ff, PROPELLER_OPERAND_DESTIMM, CCZ, PROP_2, NO_COMPRESSED, 0},
/* setdivb   000011 zcn1 cccc nnnnnnnnn 011000011 */
  {"setdivb", 0x0c4000c3, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setsqrh   000011 zcn1 cccc nnnnnnnnn 011000100 */
  {"setsqrh", 0x0c4000c4, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setsqrl   000011 zcn1 cccc nnnnnnnnn 011000101 */
  {"setsqrl", 0x0c4000c5, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setqi   000011 zcn1 cccc nnnnnnnnn 011000110 */
  {"setqi", 0x0c4000c6, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setqz   000011 zcn1 cccc nnnnnnnnn 011000111 */
  {"setqz", 0x0c4000c7, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
  
/* qlog   000011 zcn1 cccc nnnnnnnnn 011001000 */
  {"qlog", 0x0c4000c8, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* qexp   000011 zcn1 cccc nnnnnnnnn 011001001 */
  {"qexp", 0x0c4000c9, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setf   000011 zcn1 cccc nnnnnnnnn 011001010 */
  {"setf", 0x0c4000ca, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
///* settask   000011 zcn1 cccc nnnnnnnnn 011001011 */
//  {"settask", 0x0c4000cb, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* settask   000011 zcn1 cccc nnnnnnnnn 010010100 */
  {"settask", 0x0c400094, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
  
/* cfgdac0   000011 zcn1 cccc nnnnnnnnn 011001100 */
  {"cfgdac0", 0x0c4000cc, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* cfgdac1   000011 zcn1 cccc nnnnnnnnn 011001101 */
  {"cfgdac1", 0x0c4000cd, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* cfgdac2   000011 zcn1 cccc nnnnnnnnn 011001110 */
  {"cfgdac2", 0x0c4000ce, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* cfgdac3   000011 zcn1 cccc nnnnnnnnn 011001111 */
  {"cfgdac3", 0x0c4000cf, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
  
/* setdac0   000011 zcn1 cccc nnnnnnnnn 011010000 */
  {"setdac0", 0x0c4000d0, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setdac1   000011 zcn1 cccc nnnnnnnnn 011010001 */
  {"setdac1", 0x0c4000d1, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setdac2   000011 zcn1 cccc nnnnnnnnn 011010010 */
  {"setdac2", 0x0c4000d2, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setdac3   000011 zcn1 cccc nnnnnnnnn 011010011 */
  {"setdac3", 0x0c4000d3, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
  
/* cfgdacs   000011 zcn1 cccc nnnnnnnnn 011010100 */
  {"cfgdacs", 0x0c4000d4, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setdacs   000011 zcn1 cccc nnnnnnnnn 011010101 */
  {"setdacs", 0x0d0000d5, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
  
/* getp    000011 zcRi cccc ddddddddd 011010110 */
  {"getp", 0x0c4000d6, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* getnp    000011 zcRi cccc ddddddddd 011010111 */
  {"getnp", 0x0c4000d7, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
  
/* offp    000011 zcRi cccc ddddddddd 011011000 */
  {"offp", 0x0c4000d8, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* notp    000011 zcRi cccc ddddddddd 011011001 */
  {"notp", 0x0c4000d9, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* clrp    000011 zcRi cccc ddddddddd 011011010 */
  {"clrp", 0x0c4000da, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setp    000011 zcRi cccc ddddddddd 011011011 */
  {"setp", 0x0c4000db, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setpc    000011 zcRi cccc ddddddddd 011011100 */
  {"setpc", 0x0c4000dc, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setpnc    000011 zcRi cccc ddddddddd 011011101 */
  {"setpnc", 0x0c4000dd, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setpz    000011 zcRi cccc ddddddddd 011011110 */
  {"setpz", 0x0c4000de, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setpnz    000011 zcRi cccc ddddddddd 011011111 */
  {"setpnz", 0x0c4000df, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},

/* setcog    000011 zcRi cccc ddddddddd 011100000 */
  {"setcog", 0x0c4000e0, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setmap    000011 zcRi cccc ddddddddd 011100001 */
  {"setmap", 0x0c4000e1, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setquad    000011 z0Ri cccc ddddddddd 011100010 */
  {"setquad", 0x0c4000e2, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZ, PROP_2, NO_COMPRESSED, 0},
/* setquaz    000011 z1Ri cccc ddddddddd 011100010 */
  {"setquaz", 0x0d4000e2, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZ, PROP_2, NO_COMPRESSED, 0},
/* setport    000011 zcRi cccc ddddddddd 011100011 */
  {"setport", 0x0c4000e3, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setpora    000011 zcRi cccc ddddddddd 011100100 */
  {"setpora", 0x0c4000e4, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setporb    000011 zcRi cccc ddddddddd 011100101 */
  {"setporb", 0x0c4000e5, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setporc    000011 zcRi cccc ddddddddd 011100110 */
  {"setporc", 0x0c4000e6, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setpord    000011 zcRi cccc ddddddddd 011100111 */
  {"setpord", 0x0c4000e7, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
  
/* setxch    000011 zcRi cccc ddddddddd 011101000 */
  {"setxch", 0x0c4000e8, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setxfr    000011 zcRi cccc ddddddddd 011101001 */
  {"setxfr", 0x0c4000e9, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setser    000011 zcRi cccc ddddddddd 011101010 */
  {"setser", 0x0c4000ea, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setskip    000011 zcRi cccc ddddddddd 011101011 */
  {"setskip", 0x0c4000eb, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setvid    000011 zcRi cccc ddddddddd 011101100 */
  {"setvid", 0x0c4000ec, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setvidy    000011 zcRi cccc ddddddddd 011101101 */
  {"setvidy", 0x0c4000ed, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setvidi    000011 zcRi cccc ddddddddd 011101110 */
  {"setvidi", 0x0c4000ee, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setvidq    000011 zcRi cccc ddddddddd 011101111 */
  {"setvidq", 0x0c4000ef, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},

/* setctra    000011 zcRi cccc ddddddddd 011110000 */
  {"setctra", 0x0c4000f0, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setwava    000011 zcRi cccc ddddddddd 011110001 */
  {"setwava", 0x0c4000f1, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setfrqa    000011 zcRi cccc ddddddddd 011110010 */
  {"setfrqa", 0x0c4000f2, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setphsa    000011 zcRi cccc ddddddddd 011110011 */
  {"setphsa", 0x0c4000f3, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* addphsa    000011 zcRi cccc ddddddddd 011110100 */
  {"addphsa", 0x0c4000f4, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* subphsa    000011 zcRi cccc ddddddddd 011110101 */
  {"subphsa", 0x0c4000f5, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* synctra    000011 zcRi cccc ddddddddd 011110110 */
  {"syctra", 0x0c4000f6, 0xfc4001ff, PROPELLER_OPERAND_NO_OPS, CCZC, PROP_2, NO_COMPRESSED, 0},
/* capctra    000011 zcRi cccc ddddddddd 011110111 */
  {"capctra", 0x0c4000f7, 0xfc4001ff, PROPELLER_OPERAND_NO_OPS, CCZC, PROP_2, NO_COMPRESSED, 0},

/* setctrb    000011 zcRi cccc ddddddddd 011111000 */
  {"setctrb", 0x0c4000f8, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setwavb    000011 zcRi cccc ddddddddd 011111001 */
  {"setwavb", 0x0c4000f9, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setfrqb    000011 zcRi cccc ddddddddd 011111010 */
  {"setfrqb", 0x0c4000fa, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* setphsb    000011 zcRi cccc ddddddddd 011111011 */
  {"setphsb", 0x0c4000fb, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* addphsb    000011 zcRi cccc ddddddddd 011111100 */
  {"addphsb", 0x0c4000fc, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* subphsb    000011 zcRi cccc ddddddddd 011111101 */
  {"subphsb", 0x0c4000fd, 0xfc4001ff, PROPELLER_OPERAND_DESTIMM, CCZC, PROP_2, NO_COMPRESSED, 0},
/* synctrb    000011 zcRi cccc ddddddddd 011111110 */
  {"syctrb", 0x0c4000fe, 0xfc4001ff, PROPELLER_OPERAND_NO_OPS, CCZC, PROP_2, NO_COMPRESSED, 0},
/* capctrb    000011 zcRi cccc ddddddddd 011111111 */
  {"capctrb", 0x0c4000ff, 0xfc4001ff, PROPELLER_OPERAND_NO_OPS, CCZC, PROP_2, NO_COMPRESSED, 0},

/* isob    000011 zcRi cccc ddddddddd 1000bbbbb */
  {"isob", 0x0c400100, 0xfc4001e0, PROPELLER_OPERAND_BIT, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* notb    000011 zcRi cccc ddddddddd 1001bbbbb */
  {"notb", 0x0c400120, 0xfc4001e0, PROPELLER_OPERAND_BIT, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* clrb    000011 zcRi cccc ddddddddd 1010bbbbb */
  {"clrb", 0x0c400140, 0xfc4001e0, PROPELLER_OPERAND_BIT, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* setb    000011 zcRi cccc ddddddddd 1011bbbbb */
  {"setb", 0x0c400160, 0xfc4001e0, PROPELLER_OPERAND_BIT, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* setbc    000011 zcRi cccc ddddddddd 1100bbbbb */
  {"setbc", 0x0c400180, 0xfc4001e0, PROPELLER_OPERAND_BIT, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* setbnc    000011 zcRi cccc ddddddddd 1101bbbbb */
  {"setbnc", 0x0c4001a0, 0xfc4001e0, PROPELLER_OPERAND_BIT, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* setbz    000011 zcRi cccc ddddddddd 1110bbbbb */
  {"setbz", 0x0c4001c0, 0xfc4001e0, PROPELLER_OPERAND_BIT, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* setbnz    000011 zcRi cccc ddddddddd 1111bbbbb */
  {"setbnz", 0x0c4001e0, 0xfc4001e0, PROPELLER_OPERAND_BIT, CCZCWR, PROP_2, NO_COMPRESSED, 0},

/* setacca      000100 000i cccc ddddddddd sssssssss */
  {"setacca", 0x10000000, 0xff800000, PROPELLER_OPERAND_TWO_OPS, CC, PROP_2, NO_COMPRESSED, 0},
/* setaccb      000100 010i cccc ddddddddd sssssssss */
  {"setaccb", 0x11000000, 0xff800000, PROPELLER_OPERAND_TWO_OPS, CC, PROP_2, NO_COMPRESSED, 0},
/* macca      000100 100i cccc ddddddddd sssssssss */
  {"macca", 0x12000000, 0xff800000, PROPELLER_OPERAND_TWO_OPS, CC, PROP_2, NO_COMPRESSED, 0},
/* maccb      000100 110i cccc ddddddddd sssssssss */
  {"maccb", 0x13000000, 0xff800000, PROPELLER_OPERAND_TWO_OPS, CC, PROP_2, NO_COMPRESSED, 0},
/* mul      000100 zc1i cccc ddddddddd sssssssss */
  {"mul", 0x10800000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, CCZC, PROP_2, NO_COMPRESSED, 0},

/* movf      000101 000i cccc ddddddddd sssssssss */
  {"movf", 0x14000000, 0xff800000, PROPELLER_OPERAND_TWO_OPS, CC, PROP_2, NO_COMPRESSED, 0},
/* qsincos      000101 010i cccc ddddddddd sssssssss */
  {"qsincos", 0x15000000, 0xff800000, PROPELLER_OPERAND_TWO_OPS, CC, PROP_2, NO_COMPRESSED, 0},
/* qarctan      000101 100i cccc ddddddddd sssssssss */
  {"qarctan", 0x16000000, 0xff800000, PROPELLER_OPERAND_TWO_OPS, CC, PROP_2, NO_COMPRESSED, 0},
/* qrotate      000101 110i cccc ddddddddd sssssssss */
  {"qrotate", 0x17000000, 0xff800000, PROPELLER_OPERAND_TWO_OPS, CC, PROP_2, NO_COMPRESSED, 0},
/* scl      000101 zc1i cccc ddddddddd sssssssss */
  {"maccb", 0x14800000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, CCZC, PROP_2, NO_COMPRESSED, 0},

/* enc   000110 zc1i cccc ddddddddd sssssssss *//* these, */
  {"enc", 0x18800000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* jmp      000111 zc0i cccc --------- sssssssss *//* These two are in the */
  {"jmp", 0x1c000000, 0xfc83fe00, PROPELLER_OPERAND_JMP, CCZCNR, PROP_2, NO_COMPRESSED, 0},
/* jmpn     000111 zc0i cccc --------- sssssssss */
  {"jmpn", 0x1c000000, 0xfc800000, PROPELLER_OPERAND_JMPRET, CCZCNR, PROP_2, NO_COMPRESSED, 0},
/* ret      000111 zc01 cccc --------- --------- *//* wrong order either way */
  {"ret", 0x1c400000, 0xfcc00000, PROPELLER_OPERAND_NO_OPS, CCZCNR, PROP_2, NO_COMPRESSED, 0},
/* jmpret   000111 zc1i cccc ddddddddd sssssssss *//* these, */
  {"jmpret", 0x1c800000, 0xfc800000, PROPELLER_OPERAND_JMPRET, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* call     000111 zc11 cccc DDDDDDDDD sssssssss *//* too. */
  {"call", 0x1c000000, 0xfc000000, PROPELLER_OPERAND_CALL, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* jmpd      010111 zc0i cccc --------- sssssssss *//* These two are in the */
  {"jmpd", 0x5c000000, 0xfc83fe00, PROPELLER_OPERAND_JMP, CCZCNR, PROP_2, NO_COMPRESSED, 0},
/* jmpnd     010111 zc0i cccc --------- sssssssss */
  {"jmpnd", 0x5c000000, 0xfc800000, PROPELLER_OPERAND_JMPRET, CCZCNR, PROP_2, NO_COMPRESSED, 0},
/* retd      010111 zc01 cccc --------- --------- *//* wrong order either way */
  {"retd", 0x5c400000, 0xfcc00000, PROPELLER_OPERAND_NO_OPS, CCZCNR, PROP_2, NO_COMPRESSED, 0},
/* jmpretd   010111 zc1i cccc ddddddddd sssssssss *//* these, */
  {"jmpretd", 0x5c800000, 0xfc800000, PROPELLER_OPERAND_JMPRET, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* calld     010111 zc11 cccc DDDDDDDDD sssssssss *//* too. */
  {"calld", 0x5c000000, 0xfc000000, PROPELLER_OPERAND_CALL, CCZCWR, PROP_2, NO_COMPRESSED, 0},

/* setinda   111000 0000 0001 000000000 aaaaaaaaa */
  {"setinda", 0xe0040000, 0xfff7fe00, PROPELLER_OPERAND_SETINDA, 0, PROP_2, NO_COMPRESSED, 0},
/* setindb   111000 0000 0100 bbbbbbbbb 000000000 */
  {"setindb", 0xe0100000, 0xffdff1ff, PROPELLER_OPERAND_SETINDB, 0, PROP_2, NO_COMPRESSED, 0},
/* setinds   111000 0000 0101 bbbbbbbbb aaaaaaaaa */
  {"setinds", 0xe0140000, 0xffd40000, PROPELLER_OPERAND_SETINDS, 0, PROP_2, NO_COMPRESSED, 0},
/* fixinda   111000 zcri cccc ddddddddd sssssssss */
  {"fixinda", 0xe4040000, 0xfffc0000, PROPELLER_OPERAND_DESTIMM_SRCIMM, 0, PROP_2, NO_COMPRESSED, 0},
/* fixindb   111000 zcri cccc ddddddddd sssssssss */
  {"fixindb", 0xe4100000, 0xfffc0000, PROPELLER_OPERAND_DESTIMM_SRCIMM, 0, PROP_2, NO_COMPRESSED, 0},
/* fixinds   111000 zcri cccc ddddddddd sssssssss */
  {"fixinds", 0xe4140000, 0xfffc0000, PROPELLER_OPERAND_DESTIMM_SRCIMM, 0, PROP_2, NO_COMPRESSED, 0},
/* cfgpins  111010 000i cccc ddddddddd sssssssss */
  {"cfgpins", 0xe8000000, 0xff800000, PROPELLER_OPERAND_TWO_OPS, CC, PROP_2, NO_COMPRESSED, 0},
/* waitvid  111011 000i cccc ddddddddd sssssssss */
  {"waitvid", 0xec000000, 0xff800000, PROPELLER_OPERAND_TWO_OPS, CC, PROP_2, NO_COMPRESSED, 0},

/* cmpr   111000 zc0i cccc ddddddddd sssssssss */
  {"cmpr", 0xe0000000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, CCZCNR, PROP_2, NO_COMPRESSED, 0},
/* subr   111000 zc1i cccc ddddddddd sssssssss */
  {"subr", 0xe0800000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* cmpsub   111001 zc1i cccc ddddddddd sssssssss */
  {"cmpsub", 0xe4800000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, CCZCNR, PROP_2, NO_COMPRESSED, 0},
/* incmod   111010 zcri cccc ddddddddd sssssssss */
  {"incmod", 0xe8000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_2, NO_COMPRESSED, 0},
/* decmod   111011 zcri cccc ddddddddd sssssssss */
  {"decmod", 0xec000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, CCZCWR, PROP_2, NO_COMPRESSED, 0},

/* ijz     111100 00ri cccc ddddddddd sssssssss */
  {"ijz", 0xf0000000, 0xff000000, PROPELLER_OPERAND_JMPRET, CCWR, PROP_2, NO_COMPRESSED, 0},
/* ijzd     111100 01ri cccc ddddddddd sssssssss */
  {"ijzd", 0xf1000000, 0xff000000, PROPELLER_OPERAND_JMPRET, CCWR, PROP_2, NO_COMPRESSED, 0},
/* ijnz     111100 10ri cccc ddddddddd sssssssss */
  {"ijnz", 0xf2000000, 0xff000000, PROPELLER_OPERAND_JMPRET, CCWR, PROP_2, NO_COMPRESSED, 0},
/* ijnzd     111100 11ri cccc ddddddddd sssssssss */
  {"ijnzd", 0xf3000000, 0xff000000, PROPELLER_OPERAND_JMPRET, CCWR, PROP_2, NO_COMPRESSED, 0},

/* djz     111101 00ri cccc ddddddddd sssssssss */
  {"djz", 0xf4000000, 0xff000000, PROPELLER_OPERAND_JMPRET, CCWR, PROP_2, NO_COMPRESSED, 0},
/* djzd     111101 01ri cccc ddddddddd sssssssss */
  {"djzd", 0xf5000000, 0xff000000, PROPELLER_OPERAND_JMPRET, CCWR, PROP_2, NO_COMPRESSED, 0},
/* djnz     111101 10ri cccc ddddddddd sssssssss */
  {"djnz", 0xf6000000, 0xff000000, PROPELLER_OPERAND_JMPRET, CCWR, PROP_2, NO_COMPRESSED, 0},
/* djnzd     111101 11ri cccc ddddddddd sssssssss */
  {"djnzd", 0xf7000000, 0xff000000, PROPELLER_OPERAND_JMPRET, CCWR, PROP_2, NO_COMPRESSED, 0},

/* tjz     111011 00i cccc ddddddddd sssssssss */
  {"tjz", 0xf8000000, 0xff800000, PROPELLER_OPERAND_JMPRET, CC, PROP_2, NO_COMPRESSED, 0},
/* tjzd     111010 zcRi cccc ddddddddd sssssssss */
  {"tjzd", 0xf9000000, 0xff800000, PROPELLER_OPERAND_JMPRET, CC, PROP_2, NO_COMPRESSED, 0},
/* tjnz     111010 zcRi cccc ddddddddd sssssssss */
  {"tjnz", 0xfa000000, 0xff800000, PROPELLER_OPERAND_JMPRET, CC, PROP_2, NO_COMPRESSED, 0},
/* tjnzd      111110 zcRi cccc ddddddddd sssssssss */
  {"tjnzd", 0xfb000000, 0xff800000, PROPELLER_OPERAND_JMPRET, CC, PROP_2, NO_COMPRESSED, 0},

/* jp     111011 00i cccc ddddddddd sssssssss */
  {"jp", 0xf8800000, 0xff800000, PROPELLER_OPERAND_JMPRET, CC, PROP_2, NO_COMPRESSED, 0},
/* jpd     111010 zcRi cccc ddddddddd sssssssss */
  {"jpd", 0xf9800000, 0xff800000, PROPELLER_OPERAND_JMPRET, CC, PROP_2, NO_COMPRESSED, 0},
/* jnp     111010 zcRi cccc ddddddddd sssssssss */
  {"jnp", 0xfa800000, 0xff800000, PROPELLER_OPERAND_JMPRET, CC, PROP_2, NO_COMPRESSED, 0},
/* jnpd      111110 zcRi cccc ddddddddd sssssssss */
  {"jnpd", 0xfb800000, 0xff800000, PROPELLER_OPERAND_JMPRET, CC, PROP_2, NO_COMPRESSED, 0},

/* waitcnt  111111 0cri cccc ddddddddd sssssssss */
  {"waitcnt", 0xfc000000, 0xfe000000, PROPELLER_OPERAND_TWO_OPS, CCCWR, PROP_2, NO_COMPRESSED, 0},
/* waitpeq  111111 1cr0 cccc ddddddddd sssssssss */
  {"waitpeq", 0xfe000000, 0xfe800000, PROPELLER_OPERAND_TWO_OPS, CCC, PROP_2, NO_COMPRESSED, 0},
/* waitpne  111111 1cr1 cccc ddddddddd sssssssss */
  {"waitpne", 0xfe800000, 0xfe800000, PROPELLER_OPERAND_TWO_OPS, CCC, PROP_2, NO_COMPRESSED, 0},
};

const int propeller_num_opcodes =
  sizeof propeller_opcodes / sizeof propeller_opcodes[0];

const struct propeller_condition propeller_conditions[] = {
  {"if_never", 0x0 << 18, 1},
  {"if_a", 0x1 << 18, 2},
  {"if_nc_and_z", 0x2 << 18, 1},
  {"if_ae", 0x3 << 18, 2},
  {"if_c_and_nz", 0x4 << 18, 1},
  {"if_ne", 0x5 << 18, 2},
  {"if_c_ne_z", 0x6 << 18, 1},
  {"if_nc_or_nz", 0x7 << 18, 1},
  {"if_c_and_z", 0x8 << 18, 1},
  {"if_c_eq_z", 0x9 << 18, 1},
  {"if_e", 0xa << 18, 2},
  {"if_nc_or_z", 0xb << 18, 1},
  {"if_c", 0xc << 18, 2},
  {"if_c_or_nz", 0xd << 18, 1},
  {"if_be", 0xe << 18, 2},
  {"if_always", 0xf << 18, 1},
  {"if_b", 0xc << 18, 2},
  {"if_c_or_z", 0xe << 18, 1},
  {"if_nc", 0x3 << 18, 2},
  {"if_nc_and_nz", 0x1 << 18, 1},
  {"if_nz", 0x5 << 18, 2},
  {"if_nz_and_c", 0x4 << 18, 1},
  {"if_nz_and_nc", 0x1 << 18, 1},
  {"if_nz_or_c", 0xd << 18, 1},
  {"if_nz_or_nc", 0x7 << 18, 1},
  {"if_z", 0xa << 18, 2},
  {"if_z_and_c", 0x8 << 18, 1},
  {"if_z_and_nc", 0x2 << 18, 1},
  {"if_z_eq_c", 0x9 << 18, 1},
  {"if_z_ne_c", 0x6 << 18, 1},
  {"if_z_or_c", 0xe << 18, 1},
  {"if_z_or_nc", 0xb << 18, 1},
};

const int propeller_num_conditions =
  sizeof propeller_conditions / sizeof propeller_conditions[0];

const struct propeller_effect propeller_effects[] = {
  {"nr", 0, ~(1 << 23), FLAG_R},
  {"wc", 1 << 24, ~0, FLAG_C},
  {"wr", 1 << 23, ~0, FLAG_R},
  {"wz", 1 << 25, ~0, FLAG_Z},
};

const int propeller_num_effects =
  sizeof propeller_effects / sizeof propeller_effects[0];

