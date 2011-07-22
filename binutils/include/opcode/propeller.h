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

/* List of instruction formats */
#define PROPELLER_OPCODE_NO_OPS 0

struct propeller_opcode
{
  const char *name;
  int opcode;
  int mask;
  int format;
  int hardware;
};

int print_insn_propeller (bfd_vma memaddr, struct disassemble_info *info);
extern const struct propeller_opcode propeller_opcodes[];
extern const int propeller_num_opcodes;
