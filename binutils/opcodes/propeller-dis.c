/* Disassemble Propeller instructions.
   Copyright 2011 Parallax Semiconductor, Inc.

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

/* To control whether wr or nr prints as blank, and
 * whether the r bit is set by default */
#define NR 0
#define R 1

const struct propeller_opcode propeller_opcodes[] = {
/*
   mnemonic  insn  zcri cond    dst       src */
/* nop      ------ ---- cccc --------- --------- */
  {"nop", 0x00000000, 0x003c0000, PROPELLER_OPERAND_IGNORE, NR, PROP_1},
/* wrbyte   000000 zc0i cccc ddddddddd sssssssss */
  {"wrbyte", 0x00000000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, NR, PROP_1},
/* rdbyte   000000 zc1i cccc ddddddddd sssssssss */
  {"rdbyte", 0x00800000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* wrword   000001 zc0i cccc ddddddddd sssssssss */
  {"wrword", 0x04000000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, NR, PROP_1},
/* rdword   000001 zc1i cccc ddddddddd sssssssss */
  {"rdword", 0x04800000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* wrlong   000010 zc0i cccc ddddddddd sssssssss */
  {"wrlong", 0x08000000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, NR, PROP_1},
/* rdlong   000010 zc1i cccc ddddddddd sssssssss */
  {"rdlong", 0x08800000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* clkset   000011 zc01 cccc ddddddddd ------000 */
  {"clkset", 0x0c400000, 0xfcc00007, PROPELLER_OPERAND_DEST_ONLY, NR, PROP_1},
/* cogid    000011 zcr1 cccc ddddddddd ------001 */
  {"cogid", 0x0c400001, 0xfc400007, PROPELLER_OPERAND_DEST_ONLY, R, PROP_1},
/* coginit  000011 zcR1 cccc ddddddddd ------010 */
  {"coginit", 0x0c400002, 0xfc400007, PROPELLER_OPERAND_DEST_ONLY, NR,
   PROP_1},
/* cogstop  000011 zcR1 cccc ddddddddd ------011 */
  {"cogstop", 0x0c400003, 0xfc400007, PROPELLER_OPERAND_DEST_ONLY, NR,
   PROP_1},
/* locknew  000011 zcr1 cccc ddddddddd ------100 */
  {"locknew", 0x0c400004, 0xfc400007, PROPELLER_OPERAND_DEST_ONLY, R, PROP_1},
/* lockret  000011 zcR1 cccc ddddddddd ------101 */
  {"lockret", 0x0c400005, 0xfc400007, PROPELLER_OPERAND_DEST_ONLY, NR,
   PROP_1},
/* lockset  000011 zcR1 cccc ddddddddd ------110 */
  {"lockset", 0x0c400006, 0xfc400007, PROPELLER_OPERAND_DEST_ONLY, NR,
   PROP_1},
/* lockclr  000011 zcR1 cccc ddddddddd ------111 */
  {"lockclr", 0x0c400007, 0xfc400007, PROPELLER_OPERAND_DEST_ONLY, NR,
   PROP_1},
/* hubop    000011 zcRi cccc ddddddddd sssssssss */
  {"hubop", 0x0c000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, NR, PROP_1},
/* ror      001000 zcri cccc ddddddddd sssssssss */
  {"ror", 0x20000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* rol      001001 zcri cccc ddddddddd sssssssss */
  {"rol", 0x24000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* shr      001010 zcri cccc ddddddddd sssssssss */
  {"shr", 0x28000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* shl      001011 zcri cccc ddddddddd sssssssss */
  {"shl", 0x2c000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* rcr      001100 zcri cccc ddddddddd sssssssss */
  {"rcr", 0x30000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* rcl      001101 zcri cccc ddddddddd sssssssss */
  {"rcl", 0x34000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* sar      001110 zcri cccc ddddddddd sssssssss */
  {"sar", 0x38000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* rev      001111 zcri cccc ddddddddd sssssssss */
  {"rev", 0x3c000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* mins     010000 zcri cccc ddddddddd sssssssss */
  {"mins", 0x40000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* maxs     010001 zcri cccc ddddddddd sssssssss */
  {"maxs", 0x44000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* min      010010 zcri cccc ddddddddd sssssssss */
  {"min", 0x48000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* max      010011 zcri cccc ddddddddd sssssssss */
  {"max", 0x4c000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* movs     010100 zcri cccc ddddddddd sssssssss */
  {"movs", 0x50000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* movd     010101 zcri cccc ddddddddd sssssssss */
  {"movd", 0x54000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* movi     010110 zcri cccc ddddddddd sssssssss */
  {"movi", 0x58000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
  /* jmp      010111 zc0i cccc --------- sssssssss *//* These two are in the */
  {"jmp", 0x5c000000, 0xfc800000, PROPELLER_OPERAND_SOURCE_ONLY, NR, PROP_1},
  /* ret      010111 zc01 cccc --------- --------- *//* wrong order either way */
  {"ret", 0x5c400000, 0xfcc00000, PROPELLER_OPERAND_NO_OPS, NR, PROP_1},
  /* jmpret   010111 zc1i cccc ddddddddd sssssssss *//* these, */
  {"jmpret", 0x5c800000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
  /* call     010111 zc11 cccc DDDDDDDDD sssssssss *//* too. */
  {"call", 0x5c000000, 0xfc000000, PROPELLER_OPERAND_CALL, R, PROP_1},
/* test     011000 zc0i cccc ddddddddd sssssssss */
  {"test", 0x60000000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, NR, PROP_1},
/* and      011000 zc1i cccc ddddddddd sssssssss */
  {"and", 0x60000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* andn     011001 zcri cccc ddddddddd sssssssss */
  {"andn", 0x64000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* or       011010 zcri cccc ddddddddd sssssssss */
  {"or", 0x68000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* xor      011011 zcri cccc ddddddddd sssssssss */
  {"xor", 0x6c000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* muxc     011100 zcri cccc ddddddddd sssssssss */
  {"muxc", 0x70000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* muxnc    011101 zcri cccc ddddddddd sssssssss */
  {"muxnc", 0x74000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* muxz     011110 zcri cccc ddddddddd sssssssss */
  {"muxz", 0x78000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* muxnz    011111 zcri cccc ddddddddd sssssssss */
  {"muxnz", 0x7c000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* add      100000 zcri cccc ddddddddd sssssssss */
  {"add", 0x80000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* cmp      100001 zc0i cccc ddddddddd sssssssss */
  {"cmp", 0x84000000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, NR, PROP_1},
/* sub      100001 zc1i cccc ddddddddd sssssssss */
  {"sub", 0x84800000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* addabs   100010 zcri cccc ddddddddd sssssssss */
  {"addabs", 0x88000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* subabs   100011 zcri cccc ddddddddd sssssssss */
  {"subabs", 0x8c000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* sumc     100100 zcri cccc ddddddddd sssssssss */
  {"sumc", 0x90000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* sumnc    100101 zcri cccc ddddddddd sssssssss */
  {"sumnc", 0x94000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* sumz     100110 zcri cccc ddddddddd sssssssss */
  {"sumz", 0x98000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* sumnz    100111 zcri cccc ddddddddd sssssssss */
  {"sumnz", 0x9c000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* mov      101000 zcri cccc ddddddddd sssssssss */
  {"mov", 0xa0000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* neg      101001 zcri cccc ddddddddd sssssssss */
  {"neg", 0xa4000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* abs      101010 zcri cccc ddddddddd sssssssss */
  {"abs", 0xa8000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* absneg   101011 zcri cccc ddddddddd sssssssss */
  {"absneg", 0xac000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* negc     101100 zcri cccc ddddddddd sssssssss */
  {"negc", 0xb0000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* negnc    101101 zcri cccc ddddddddd sssssssss */
  {"negnc", 0xb4000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* negz     101110 zcri cccc ddddddddd sssssssss */
  {"negz", 0xb8000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* negnz    101111 zcri cccc ddddddddd sssssssss */
  {"negnz", 0xbc000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* cmps     110000 zcRi cccc ddddddddd sssssssss */
  {"cmps", 0xc0000000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, NR, PROP_1},
/* cmpsx    110001 zcRi cccc ddddddddd sssssssss */
  {"cmpsx", 0xc4000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, NR, PROP_1},
/* addx     110010 zcri cccc ddddddddd sssssssss */
  {"addx", 0xc8000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* cmpx     110011 zc0i cccc ddddddddd sssssssss */
  {"cmpx", 0xcc000000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, NR, PROP_1},
/* subx     110011 zc1i cccc ddddddddd sssssssss */
  {"subx", 0xcc800000, 0xfc800000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* adds     110100 zcri cccc ddddddddd sssssssss */
  {"adds", 0xd0000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* subs     110101 zcri cccc ddddddddd sssssssss */
  {"subs", 0xd4000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* addsx    110110 zcri cccc ddddddddd sssssssss */
  {"addsx", 0xd8000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* subsx    110111 zcri cccc ddddddddd sssssssss */
  {"subsx", 0xdc000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* cmpsub   111000 zcri cccc ddddddddd sssssssss */
  {"cmpsub", 0xe0000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* djnz     111001 zcri cccc ddddddddd sssssssss */
  {"djnz", 0xe4000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* tjnz     111010 zcRi cccc ddddddddd sssssssss */
  {"tjnz", 0xe8000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, NR, PROP_1},
/* tjz      111011 zcRi cccc ddddddddd sssssssss */
  {"tjz", 0xec000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, NR, PROP_1},
/* waitpeq  111100 zcRi cccc ddddddddd sssssssss */
  {"waitpeq", 0xf0000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, NR, PROP_1},
/* waitpne  111101 zcRi cccc ddddddddd sssssssss */
  {"waitpne", 0xf4000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, NR, PROP_1},
/* waitcnt  111110 zcri cccc ddddddddd sssssssss */
  {"waitcnt", 0xf8000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, R, PROP_1},
/* waitvid  111111 zcRi cccc ddddddddd sssssssss */
  {"waitvid", 0xfc000000, 0xfc000000, PROPELLER_OPERAND_TWO_OPS, NR, PROP_1},
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

static int
read_word (bfd_vma memaddr, int *word, disassemble_info * info)
{
  int status;
  bfd_byte x[4];

  status = (*info->read_memory_func) (memaddr, x, 4, info);
  if (status != 0)
    return -1;

  *word = x[0] | x[1] << 8 | x[2] << 16 | x[3] << 24;
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
	      }
	    info->target = src<<2;
	    (*info->print_address_func) (info->target, info);
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
	    FPRINTF (F, OP.name);
	    FPRINTF (F, AFTER_INSTRUCTION);
	      {
		info->target = dst<<2;
		(*info->print_address_func) (info->target, info);
	      }
	    FPRINTF (F, OPERAND_SEPARATOR);
	    if (immediate)
	      {
	      FPRINTF (F, "#");
	      FPRINTF (F, "%d", src);
	      }
	    else
	      {
		info->target = src<<2;
		(*info->print_address_func) (info->target, info);
	      }
	    goto done;
	  default:
	    /* TODO: is this a proper way of signalling an error? */
	    FPRINTF (F, "<internal error: unrecognized instruction type>");
	    return -1;
	  }
    }
done:
  FPRINTF (F, " %s", flags[set + OP.result * 8]);

#undef OP
  return memaddr - start_memaddr;
}
