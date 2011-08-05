/* Disassemble Propeller instructions.
   Copyright 2011 Parallax Semiconductor, Inc.

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
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include <stdio.h>
#include "sysdep.h"
#include "dis-asm.h"
#include "opcode/propeller.h"

const struct propeller_opcode propeller_opcodes[] = {
  { "abs",	0xa8800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "absneg",	0xac800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "add",	0x80800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "addabs",	0x88800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "adds",	0xd0800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "addsx",	0xd8800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "addx",	0xc8800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "and",	0x60800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "andn",	0x64800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "call",	0x5cc00000, 0xffc00000, PROPELLER_OPERAND_SOURCE_ONLY,	PROP_1 },
  { "clkset",	0x0c400000, 0xffc00007, PROPELLER_OPERAND_DEST_ONLY,	PROP_1 },
  { "cmp",	0x84000000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "cmps",	0xc0000000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "cmpsub",	0xe0000000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "cmpsx",	0xc4000000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "cmpx",	0xcc000000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "cogid",	0x0cc00001, 0xffc00007, PROPELLER_OPERAND_DEST_ONLY,	PROP_1 },
  { "coginit",	0x0c400002, 0xffc00007, PROPELLER_OPERAND_DEST_ONLY,	PROP_1 },
  { "cogstop",	0x0c400003, 0xffc00007, PROPELLER_OPERAND_DEST_ONLY,	PROP_1 },
  { "djnz",	0xe4800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "hubop",	0x0c000000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "jmp",	0x5c000000, 0xff800000, PROPELLER_OPERAND_SOURCE_ONLY,	PROP_1 },
  { "jmpret",	0x5c800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "lockclr",	0x0c400007, 0xffc00007, PROPELLER_OPERAND_DEST_ONLY,	PROP_1 },
  { "locknew",	0x0cc00004, 0xffc00007, PROPELLER_OPERAND_DEST_ONLY,	PROP_1 },
  { "lockret",	0x0c400005, 0xffc00007, PROPELLER_OPERAND_DEST_ONLY,	PROP_1 },
  { "lockset",	0x0c400006, 0xffc00007, PROPELLER_OPERAND_DEST_ONLY,	PROP_1 },
  { "max",	0x4c800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "maxs",	0x44800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "min",	0x48800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "mins",	0x40800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "mov",	0xa0800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "movd",	0x54800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "movi",	0x58800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "movs",	0x50800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "muxc",	0x70800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "muxnc",	0x74800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "muxnz",	0x7c800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "muxz",	0x78800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "neg",	0xa4800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "negc",	0xb0800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "negnc",	0xb4800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "negnz",	0xbc800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "negz",	0xb8800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "nop",	0x00000000, 0x003c0000, PROPELLER_OPERAND_NO_OPS,	PROP_1 },
  { "or",	0x68800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "rdbyte",	0x00800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "rdlong",	0x08800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "rdword",	0x04800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "rcl",	0x34800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "rcr",	0x30800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "ret",	0x5c400000, 0xffc00000, PROPELLER_OPERAND_NO_OPS,	PROP_1 },
  { "rev",	0x3c800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "rol",	0x44800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "ror",	0x40800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "sar",	0x38800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "shl",	0x2c800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "shr",	0x28800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "sub",	0x84800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "subabs",	0x8c800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "subs",	0xd4800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "subsx",	0xdc800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "subx",	0xcc800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "sumc",	0x90800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "sumnc",	0x94800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "sumnz",	0x9c800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "sumz",	0x98800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "test",	0x60000000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "tjnz",	0xe8000000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "tjz",	0xec000000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "waitcnt",	0xf8800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "waitpeq",	0xf0000000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "waitpne",	0xf4000000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "waitvid",	0xfc000000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "wrbyte",	0x00000000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "wrlong",	0x08000000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "wrword",	0x04000000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
  { "xor",	0x6c800000, 0xff800000, PROPELLER_OPERAND_TWO_OPS,	PROP_1 },
};

const int propeller_num_opcodes = sizeof propeller_opcodes / sizeof propeller_opcodes[0];

const struct propeller_condition propeller_conditions[] = {
  { "if_always",    0xf << 18 },
  { "if_never",     0x0 << 18 },
  { "if_e",         0xa << 18 },
  { "if_ne",        0x5 << 18 },
  { "if_a",         0x1 << 18 },
  { "if_b",         0xc << 18 },
  { "if_ae",        0x3 << 18 },
  { "if_be",        0xe << 18 },
  { "if_c",         0xc << 18 },
  { "if_nc",        0x3 << 18 },
  { "if_z",         0xa << 18 },
  { "if_nz",        0x5 << 18 },
  { "if_c_eq_z",    0x9 << 18 },
  { "if_c_ne_z",    0x6 << 18 },
  { "if_c_and_z",   0x8 << 18 },
  { "if_c_and_nz",  0x4 << 18 },
  { "if_nc_and_z",  0x2 << 18 },
  { "if_nc_and_nz", 0x1 << 18 },
  { "if_c_or_z",    0xe << 18 },
  { "if_c_or_nz",   0xd << 18 },
  { "if_nc_or_z",   0xb << 18 },
  { "if_nc_or_nz",  0x7 << 18 },
  { "if_z_eq_c",    0x9 << 18 },
  { "if_z_ne_c",    0x6 << 18 },
  { "if_z_and_c",   0x8 << 18 },
  { "if_z_and_nc",  0x2 << 18 },
  { "if_nz_and_c",  0x4 << 18 },
  { "if_nz_and_nc", 0x1 << 18 },
  { "if_z_or_c",    0xe << 18 },
  { "if_z_or_nc",   0xb << 18 },
  { "if_nz_or_c",   0xd << 18 },
  { "if_nz_or_nc",  0x7 << 18 },
};

const int propeller_num_conditions = sizeof propeller_conditions / sizeof propeller_conditions[0];

const struct propeller_effect propeller_effects[] = {
  { "wc", 1 << 24, ~0 },
  { "wz", 1 << 25, ~0 },
  { "nr", 0,       ~(1 << 23) },
  { "wr", 1 << 23, ~0 }
};

const int propeller_num_effects = sizeof propeller_effects / sizeof propeller_effects[0];

int
print_insn_propeller (bfd_vma memaddr, struct disassemble_info *info)
{
  (void)memaddr;
  (void)info;
  return 0;
}
