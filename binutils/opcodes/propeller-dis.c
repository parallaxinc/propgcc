/* Disassemble Propeller instructions.
   Copyright 2011 Parallax Inc.

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABIITY
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


static int
read_word (bfd_vma memaddr, int *word, disassemble_info * info)
{
  int status;
  bfd_byte x[4];

  status = (*info->read_memory_func) (memaddr, x, 4, info);
  if (status != 0)
    return -1;

  *word = x[3] << 24 | x[2] << 16 | x[1] << 8 | x[0];
  return 0;
}

/* Print the Propeller instruction at address MEMADDR in debugged memory,
   on INFO->STREAM.  Returns length of the instruction, in bytes.  */

#define AFTER_INSTRUCTION	"\t"
#define OPERAND_SEPARATOR	", "
#define FPRINTF	(*info->fprintf_func)
#define F	info->stream

static const char *const flags[16] = {
  "",
  "wr",
  "wc",
  "wc, wr",
  "wz",
  "wz, wr",
  "wz, wc",
  "wz, wc, wr",
  "nr",
  "",
  "wc, nr",
  "wc",
  "wz, nr",
  "wz",
  "wz, wc, nr",
  "wz, wc"
};

int
print_insn_propeller (bfd_vma memaddr, struct disassemble_info *info)
{
  bfd_vma start_memaddr = memaddr;
  int opcode;
  int src, dst;
  int condition;
  int set, immediate;
  int i;

  info->bytes_per_line = 4;
  info->bytes_per_chunk = 4;
  info->display_endian = BFD_ENDIAN_LITTLE;

  if (read_word (memaddr, &opcode, info) != 0)
    return -1;
  memaddr += 4;

  src = (opcode >> 0) & 0x1ff;
  dst = (opcode >> 9) & 0x1ff;
  condition = (opcode >> 18) & 0xf;
  immediate = (opcode >> 22) & 1;
  set = (opcode >> 23) & 7;

  if (condition == 0)
    {
      FPRINTF (F, "\t\tnop");
      return 4;
    }
  else if (condition != 15)
    {
      FPRINTF (F, propeller_conditions[condition].name);
      for (i = 0; i < propeller_conditions[condition].tabs; i++)
	{
	  FPRINTF (F, "\t");
	}
    }
  else
    {
      FPRINTF (F, "\t\t");
    }

  for (i = 0; i < propeller_num_opcodes; i++)
    {
#define OP propeller_opcodes[i]
      if ((opcode & OP.mask) == OP.opcode)
	switch (OP.format)
	  {
	  case PROPELLER_OPERAND_NO_OPS:
	    FPRINTF (F, OP.name);
	    goto done;
	  case PROPELLER_OPERAND_SOURCE_ONLY:
	    FPRINTF (F, OP.name);
	    FPRINTF (F, AFTER_INSTRUCTION);
	    if (immediate)
	      {
		FPRINTF (F, "#");
		FPRINTF (F, "%d", src);
	      }
	    else
	      {
		info->target = src;
		(*info->print_address_func) (info->target, info);
	      }
	    goto done;
	  case PROPELLER_OPERAND_JMP:
	    FPRINTF (F, OP.name);
	    FPRINTF (F, AFTER_INSTRUCTION);
	    {
	      if (immediate) FPRINTF (F, "#");
	      info->target = src<<2;
	      (*info->print_address_func) (info->target, info);
	    }
	    goto done;
	  case PROPELLER_OPERAND_DEST_ONLY:
	    FPRINTF (F, OP.name);
	    FPRINTF (F, AFTER_INSTRUCTION);
	    {
	      info->target = dst<<2;
	      (*info->print_address_func) (info->target, info);
	    }
	    goto done;
	  case PROPELLER_OPERAND_TWO_OPS:
	  case PROPELLER_OPERAND_JMPRET:
	  case PROPELLER_OPERAND_MOVA:
	    FPRINTF (F, OP.name);
	    FPRINTF (F, AFTER_INSTRUCTION);
	      {
		info->target = dst<<2;
		(*info->print_address_func) (info->target, info);
	      }
	    FPRINTF (F, OPERAND_SEPARATOR);
	    if (immediate && (OP.format != PROPELLER_OPERAND_JMPRET && OP.format != PROPELLER_OPERAND_MOVA))
	      {
	      FPRINTF (F, "#");
	      FPRINTF (F, "%d", src);
	      }
	    else
	      {
		if (immediate) FPRINTF (F, "#");
		info->target = src<<2;
		(*info->print_address_func) (info->target, info);
	      }
	    goto done;
	  case PROPELLER_OPERAND_BRS:
	    if (!immediate)
	      continue;
	    if (dst != 17) /* not the PC */
	      continue;
	    if (memaddr < 2048)
	      continue; /* could be in COG memory */
	    FPRINTF (F, OP.name);
	    FPRINTF (F, AFTER_INSTRUCTION);
	    if ((unsigned)OP.opcode == 0x80000000U)
	      info->target = memaddr + src;
	    else
	      info->target = memaddr - src;
	    (*info->print_address_func) (info->target, info);
	    goto done;
	  case PROPELLER_OPERAND_XMMIO:
	  case PROPELLER_OPERAND_LDI:
	  case PROPELLER_OPERAND_BRW:
	    /* disassembly not implemented yet */
	    continue;
	  default:
	    /* TODO: is this a proper way of signalling an error? */
	    FPRINTF (F, "<internal error: unrecognized instruction type>");
	    return -1;
	  }
    }
done:
  if (i < propeller_num_opcodes)
    {
      FPRINTF (F, " %s", flags[set + OP.result * 8]);
    }
  else
    {
      FPRINTF (F, "<unrecognized instruction>");
    }

#undef OP
  return memaddr - start_memaddr;
}
