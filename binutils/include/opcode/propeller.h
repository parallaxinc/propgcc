/* Parallax Propeller opcde list.
   Copyright 2011 Parallax Semiconductor, Inc.

   This file is part of GDB and GAS.

   GDB and GAS are free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GDB and GAS are distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GDB or GAS; see the file COPYING3.  If not, write to
   the Free Software Foundation, 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* List of possible hardware types, including any pseudo-types like
 * LMM on Propeller-1, */
#define PROP_1 0
#define PROP_2 1
#define PROP_1_LMM 2

/* List of instruction formats */
#define PROPELLER_OPERAND_NO_OPS      0
#define PROPELLER_OPERAND_SOURCE_ONLY 1
#define PROPELLER_OPERAND_DEST_ONLY   2
#define PROPELLER_OPERAND_TWO_OPS     3
#define PROPELLER_OPERAND_CALL        4
#define PROPELLER_OPERAND_IGNORE      5
#define PROPELLER_OPERAND_JMP         6
#define PROPELLER_OPERAND_JMPRET      7
#define PROPELLER_OPERAND_MOVA        8
#define PROPELLER_OPERAND_LDI         9

struct propeller_opcode
{
  const char *name;
  int opcode;
  int mask;
  int format;
  int result;
  int hardware;
};

struct propeller_condition
{
  const char *name;
  int value;
  int tabs;
};

struct propeller_effect
{
  const char *name;
  int or;
  int and;
};


int print_insn_propeller (bfd_vma memaddr, struct disassemble_info *info);
extern const struct propeller_opcode propeller_opcodes[];
extern const int propeller_num_opcodes;
extern const struct propeller_condition propeller_conditions[];
extern const int propeller_num_conditions;
extern const struct propeller_effect propeller_effects[];
extern const int propeller_num_effects;
