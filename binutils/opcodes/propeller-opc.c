/* Opcode table for Parallax Propeller
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
   along with this file; see the file COPYING.  If not, write to the
   Free Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include <stdio.h>
#include "sysdep.h"
#include "dis-asm.h"
#include "opcode/propeller.h"

const struct propeller_opcode propeller_opcodes[] = {
  /* name,      pattern, mask,  opcode type,            insn type,    alias */
};

#define NUM_PROPELLER_OPCODES (sizeof(propeller_opcodes)/sizeof(struct propeller_opcode))
