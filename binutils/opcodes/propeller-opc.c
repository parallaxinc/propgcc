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

#include <stdio.h>
#include "sysdep.h"
#include "dis-asm.h"
#include "opcode/propeller.h"

/* To control whether wr or nr prints as blank, and
 * whether the r bit is set by default */
#define NR 0
#define R 1

const struct propeller_opcode propeller_opcodes[] = {
/*
   mnemonic  insn  zcri cond    dst       src */
/* nop      ------ ---- cccc --------- --------- */
  {"nop", 0x00000000, 0x003c0000, PROPELLER_OPERAND_IGNORE, NR, PROP_1, COMPRESS_MACRO, 0x00},

  /* we put the pseudo-instructions here so the disassembler gets a first
     crack at them
  */
  /* brs is a fake instruction that expands to either add or sub of a pc relative offset */
/* brs      100000 zcri cccc ddddddddd sssssssss */
  {"brs", 0x80000000, 0xfc000000, PROPELLER_OPERAND_BRS, R, PROP_1_LMM, COMPRESS_BRL, 0},
  /* dummy entry for the disassembler only */
  {"brs ", 0x84800000, 0xfc800000, PROPELLER_OPERAND_BRS, R, PROP_1_LMM, COMPRESS_BRL, 0},

/* ldi is a fake instruction built from a rdlong and a constant that decodes as NOP */
/* ldi      000010 zc1i cccc ddddddddd sssssssss */
  {"ldi", 0x08800000, 0xfc800000, PROPELLER_OPERAND_LDI, R, PROP_1_LMM, NO_COMPRESSED, 0},

/* brw is also made of rdlong and a constant.  We may shrink it later. */
/* brw       000010 zc1i cccc ddddddddd sssssssss */
  {"brw", 0x08800000, 0xfc800000, PROPELLER_OPERAND_BRW, R, PROP_1_LMM, NO_COMPRESSED, 0},

/* xmmio is made up of a mov immediate followed by a call; the first part
   is mov, so that's what we give here
*/
/* xmmio      101000 zcri cccc ddddddddd sssssssss */
  {"xmmio", 0xa0000000, 0xfc000000, PROPELLER_OPERAND_XMMIO, R, PROP_1_LMM, NO_COMPRESSED, 0},


/* fcache is a jmp followed by a 32 bit constant */
  {"fcache", 0x5c000000, 0xffffffff, PROPELLER_OPERAND_FCACHE, NR, PROP_1_LMM, COMPRESS_MACRO, MACRO_FCACHE},
/* mvi expands to a jmp followed by a 32 bit constant, just like fcache */
  {"mvi", 0x5c000000, 0xffffffff, PROPELLER_OPERAND_MVI, NR, PROP_1_LMM, COMPRESS_MVI, PREFIX_MVI},
/* mviw expands to a jmp followed by a 32 bit constant, just like fcache */
  {"mviw", 0x5c000000, 0xffffffff, PROPELLER_OPERAND_MVI, NR, PROP_1_LMM, COMPRESS_MVIW, PREFIX_MVIW},
/* lcall expands to a jmp followed by a 32 bit constant, just like fcache */
  {"lcall", 0x5c000000, 0xffffffff, PROPELLER_OPERAND_LCALL, NR, PROP_1_LMM, COMPRESS_MACRO, MACRO_LCALL},

/* lcall expands to a jmp followed by a 32 bit constant, just like fcache */
  {"lret", 0x5c800000, 0xffffffff, PROPELLER_OPERAND_MACRO_0, NR, PROP_1_LMM, COMPRESS_MACRO, MACRO_RET},
  {"lmul", 0x5c800000, 0xffffffff, PROPELLER_OPERAND_MACRO_0, NR, PROP_1_LMM, COMPRESS_MACRO, MACRO_MUL},
  {"ludiv", 0x5c800000, 0xffffffff, PROPELLER_OPERAND_MACRO_0, NR, PROP_1_LMM, COMPRESS_MACRO, MACRO_UDIV},
  {"ldiv", 0x5c800000, 0xffffffff, PROPELLER_OPERAND_MACRO_0, NR, PROP_1_LMM, COMPRESS_MACRO, MACRO_DIV},

/* pushm and popm expand to a mov followed by a jmpret */
/* push and pop      101000 zcri cccc ddddddddd sssssssss */
  {"lpushm", 0xa0000000, 0xffffffff, PROPELLER_OPERAND_MACRO_8, R, PROP_1_LMM, COMPRESS_MACRO, MACRO_PUSHM},
  {"lpopm", 0xa0000000, 0xffffffff, PROPELLER_OPERAND_MACRO_8, R, PROP_1_LMM, COMPRESS_MACRO, MACRO_POPM},

/* leasp expands to a mov rN,sp followed by add rN,#x */
/* it's mainly intended for compressed mode, but we have to support the
   expanded version for fcache */
/* leasp      101000 zcri cccc ddddddddd sssssssss */
  {"leasp", 0xa0000000, 0xffffffff, PROPELLER_OPERAND_LEASP, R, PROP_1_LMM, COMPRESS_MACRO, PREFIX_LEASP},

/* wrbyte   000000 zc0i cccc ddddddddd sssssssss */
  {"wrbyte", 0x00000000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, NR, PROP_1, COMPRESS_XOP, XOP_WRB},
/* rdbyte   000000 zc1i cccc ddddddddd sssssssss */
  {"rdbyte", 0x00800000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, COMPRESS_XOP, XOP_RDB},
/* wrword   000001 zc0i cccc ddddddddd sssssssss */
  {"wrword", 0x04000000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, NR, PROP_1, NO_COMPRESSED, 0},
/* rdword   000001 zc1i cccc ddddddddd sssssssss */
  {"rdword", 0x04800000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* wrlong   000010 zc0i cccc ddddddddd sssssssss */
  {"wrlong", 0x08000000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, NR, PROP_1, COMPRESS_XOP, XOP_WRL},
/* rdlong   000010 zc1i cccc ddddddddd sssssssss */
  {"rdlong", 0x08800000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, COMPRESS_XOP, XOP_RDL},
/* clkset   000011 zc01 cccc ddddddddd ------000 */
  {"clkset", 0x0c400000, 0xfcc00007, PROPELLER_OPERAND_DEST_ONLY, NR, PROP_1, NO_COMPRESSED, 0},
/* cogid    000011 zcr1 cccc ddddddddd ------001 */
  {"cogid", 0x0c400001, 0xfc400007, PROPELLER_OPERAND_DEST_ONLY, R, PROP_1, NO_COMPRESSED, 0},
/* coginit  000011 zcR1 cccc ddddddddd ------010 */
  {"coginit", 0x0c400002, 0xfc400007, PROPELLER_OPERAND_DEST_ONLY, NR, PROP_1, NO_COMPRESSED, 0},
/* cogstop  000011 zcR1 cccc ddddddddd ------011 */
  {"cogstop", 0x0c400003, 0xfc400007, PROPELLER_OPERAND_DEST_ONLY, NR, PROP_1, NO_COMPRESSED, 0},
/* locknew  000011 zcr1 cccc ddddddddd ------100 */
  {"locknew", 0x0c400004, 0xfc400007, PROPELLER_OPERAND_DEST_ONLY, R, PROP_1, NO_COMPRESSED, 0},
/* lockret  000011 zcR1 cccc ddddddddd ------101 */
  {"lockret", 0x0c400005, 0xfc400007, PROPELLER_OPERAND_DEST_ONLY, NR, PROP_1, NO_COMPRESSED, 0},
/* lockset  000011 zcR1 cccc ddddddddd ------110 */
  {"lockset", 0x0c400006, 0xfc400007, PROPELLER_OPERAND_DEST_ONLY, NR, PROP_1, NO_COMPRESSED, 0},
/* lockclr  000011 zcR1 cccc ddddddddd ------111 */
  {"lockclr", 0x0c400007, 0xfc400007, PROPELLER_OPERAND_DEST_ONLY, NR, PROP_1, NO_COMPRESSED, 0},
/* hubop    000011 zcRi cccc ddddddddd sssssssss */
  {"hubop", 0x0c000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, NR, PROP_1, NO_COMPRESSED, 0},
/* ror      001000 zcri cccc ddddddddd sssssssss */
  {"ror", 0x20000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* rol      001001 zcri cccc ddddddddd sssssssss */
  {"rol", 0x24000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* shr      001010 zcri cccc ddddddddd sssssssss */
  {"shr", 0x28000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, COMPRESS_XOP, XOP_SHR},
/* shl      001011 zcri cccc ddddddddd sssssssss */
  {"shl", 0x2c000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, COMPRESS_XOP, XOP_SHL},
/* rcr      001100 zcri cccc ddddddddd sssssssss */
  {"rcr", 0x30000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* rcl      001101 zcri cccc ddddddddd sssssssss */
  {"rcl", 0x34000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* sar      001110 zcri cccc ddddddddd sssssssss */
  {"sar", 0x38000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, COMPRESS_XOP, XOP_SAR},
/* rev      001111 zcri cccc ddddddddd sssssssss */
  {"rev", 0x3c000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* mins     010000 zcri cccc ddddddddd sssssssss */
  {"mins", 0x40000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* maxs     010001 zcri cccc ddddddddd sssssssss */
  {"maxs", 0x44000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* min      010010 zcri cccc ddddddddd sssssssss */
  {"min", 0x48000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* max      010011 zcri cccc ddddddddd sssssssss */
  {"max", 0x4c000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* movs     010100 zcri cccc ddddddddd sssssssss */
  {"movs", 0x50000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* movd     010101 zcri cccc ddddddddd sssssssss */
  {"movd", 0x54000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* movi     010110 zcri cccc ddddddddd sssssssss */
  {"movi", 0x58000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* jmp      010111 zc0i cccc --------- sssssssss *//* These two are in the */
  {"jmp", 0x5c000000, 0xfc83fe00, PROPELLER_OPERAND_JMP, NR, PROP_1, NO_COMPRESSED, 0},
/* jmpn     010111 zc0i cccc --------- sssssssss */
  {"jmpn", 0x5c000000, 0xfc800000, PROPELLER_OPERAND_JMPRET, NR, PROP_1, NO_COMPRESSED, 0},
/* ret      010111 zc01 cccc --------- --------- *//* wrong order either way */
  {"ret", 0x5c400000, 0xfcc00000, PROPELLER_OPERAND_NO_OPS, NR, PROP_1, NO_COMPRESSED, 0},
/* jmpret   010111 zc1i cccc ddddddddd sssssssss *//* these, */
  {"jmpret", 0x5c800000, 0xfc800000, PROPELLER_OPERAND_JMPRET, R, PROP_1, NO_COMPRESSED, 0},
/* call     010111 zc11 cccc DDDDDDDDD sssssssss *//* too. */
  {"call", 0x5c000000, 0xfc000000, PROPELLER_OPERAND_CALL, R, PROP_1, NO_COMPRESSED, 0},
/* test     011000 zcRi cccc ddddddddd sssssssss */
  {"test", 0x60000000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, NR, PROP_1, NO_COMPRESSED, 0},
/* and      011000 zcri cccc ddddddddd sssssssss */
  {"and", 0x60000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, COMPRESS_XOP, XOP_AND},
/* andn     011001 zcri cccc ddddddddd sssssssss */
  {"andn", 0x64000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, COMPRESS_XOP, XOP_ANDN},
/* or       011010 zcri cccc ddddddddd sssssssss */
  {"or", 0x68000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, COMPRESS_XOP, XOP_OR},
/* xor      011011 zcri cccc ddddddddd sssssssss */
  {"xor", 0x6c000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, COMPRESS_XOP, XOP_XOR},
/* muxc     011100 zcri cccc ddddddddd sssssssss */
  {"muxc", 0x70000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* muxnc    011101 zcri cccc ddddddddd sssssssss */
  {"muxnc", 0x74000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* muxz     011110 zcri cccc ddddddddd sssssssss */
  {"muxz", 0x78000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* muxnz    011111 zcri cccc ddddddddd sssssssss */
  {"muxnz", 0x7c000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* add      100000 zcri cccc ddddddddd sssssssss */
  {"add", 0x80000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, COMPRESS_XOP, XOP_ADD},
/* cmp      100001 zc0i cccc ddddddddd sssssssss */
  {"cmp", 0x84000000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, NR, PROP_1, NO_COMPRESSED, 0},
/* sub      100001 zc1i cccc ddddddddd sssssssss */
  {"sub", 0x84800000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, COMPRESS_XOP, XOP_SUB},
/* addabs   100010 zcri cccc ddddddddd sssssssss */
  {"addabs", 0x88000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* subabs   100011 zcri cccc ddddddddd sssssssss */
  {"subabs", 0x8c000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* sumc     100100 zcri cccc ddddddddd sssssssss */
  {"sumc", 0x90000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* sumnc    100101 zcri cccc ddddddddd sssssssss */
  {"sumnc", 0x94000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* sumz     100110 zcri cccc ddddddddd sssssssss */
  {"sumz", 0x98000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* sumnz    100111 zcri cccc ddddddddd sssssssss */
  {"sumnz", 0x9c000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* mov      101000 zcri cccc ddddddddd sssssssss */
  {"mov", 0xa0000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, COMPRESS_MOV, PREFIX_MVIB},
/* mova is like mov, but it assumes the immediate operand is a cog address */
/* mova      101000 zcri cccc ddddddddd sssssssss */
  {"mova", 0xa0000000, 0xfc000000, PROPELLER_OPERAND_MOVA, R, PROP_1, NO_COMPRESSED, 0},
/* neg      101001 zcri cccc ddddddddd sssssssss */
  {"neg", 0xa4000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, COMPRESS_XOP, XOP_NEG},
/* abs      101010 zcri cccc ddddddddd sssssssss */
  {"abs", 0xa8000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* absneg   101011 zcri cccc ddddddddd sssssssss */
  {"absneg", 0xac000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* negc     101100 zcri cccc ddddddddd sssssssss */
  {"negc", 0xb0000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* negnc    101101 zcri cccc ddddddddd sssssssss */
  {"negnc", 0xb4000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* negz     101110 zcri cccc ddddddddd sssssssss */
  {"negz", 0xb8000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* negnz    101111 zcri cccc ddddddddd sssssssss */
  {"negnz", 0xbc000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* cmps     110000 zcRi cccc ddddddddd sssssssss */
  {"cmps", 0xc0000000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, NR, PROP_1, COMPRESS_XOP, XOP_CMPS},
/* cmpsx    110001 zcRi cccc ddddddddd sssssssss */
  {"cmpsx", 0xc4000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, NR, PROP_1, NO_COMPRESSED, 0},
/* addx     110010 zcri cccc ddddddddd sssssssss */
  {"addx", 0xc8000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* cmpx     110011 zc0i cccc ddddddddd sssssssss */
  {"cmpx", 0xcc000000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, NR, PROP_1, NO_COMPRESSED, 0},
/* subx     110011 zc1i cccc ddddddddd sssssssss */
  {"subx", 0xcc800000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* adds     110100 zcri cccc ddddddddd sssssssss */
  {"adds", 0xd0000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* subs     110101 zcri cccc ddddddddd sssssssss */
  {"subs", 0xd4000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* addsx    110110 zcri cccc ddddddddd sssssssss */
  {"addsx", 0xd8000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* subsx    110111 zcri cccc ddddddddd sssssssss */
  {"subsx", 0xdc000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* cmpsub   111000 zcri cccc ddddddddd sssssssss */
  {"cmpsub", 0xe0000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* djnz     111001 zcri cccc ddddddddd sssssssss */
  {"djnz", 0xe4000000, 0xfc000000, PROPELLER_OPERAND_JMPRET, R, PROP_1, NO_COMPRESSED, 0},
/* tjnz     111010 zcRi cccc ddddddddd sssssssss */
  {"tjnz", 0xe8000000, 0xfc000000, PROPELLER_OPERAND_JMPRET, NR, PROP_1, NO_COMPRESSED, 0},
/* tjz      111011 zcRi cccc ddddddddd sssssssss */
  {"tjz", 0xec000000, 0xfc000000, PROPELLER_OPERAND_JMPRET, NR, PROP_1, NO_COMPRESSED, 0},
/* waitpeq  111100 zcRi cccc ddddddddd sssssssss */
  {"waitpeq", 0xf0000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, NR, PROP_1, NO_COMPRESSED, 0},
/* waitpne  111101 zcRi cccc ddddddddd sssssssss */
  {"waitpne", 0xf4000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, NR, PROP_1, NO_COMPRESSED, 0},
/* waitcnt  111110 zcri cccc ddddddddd sssssssss */
  {"waitcnt", 0xf8000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1, NO_COMPRESSED, 0},
/* waitvid  111111 zcRi cccc ddddddddd sssssssss */
  {"waitvid", 0xfc000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, NR, PROP_1, NO_COMPRESSED, 0},

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
  {"nr", 0, ~(1 << 23)},
  {"wc", 1 << 24, ~0},
  {"wr", 1 << 23, ~0},
  {"wz", 1 << 25, ~0},
};

const int propeller_num_effects =
  sizeof propeller_effects / sizeof propeller_effects[0];

