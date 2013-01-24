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

#include "elf-bfd.h"
#include "elf/internal.h"
#include "elf/propeller.h"

/*
 * some flags
 */
static int is_p2;  /* if set, assume p2 instructions */
static int is_compress; /* if set, assume compressed instructions */
static int is_nocompress; /* if set, assume uncompressed instructions */


static void
parse_prop_dis_option (const char *option, unsigned int len)
{
  if (len == 2 && !strncmp (option, "p2", len))
    {
      is_p2 = 1;
      return;
    }
  if (len == 8 && !strncmp (option, "compress", len))
    {
      is_compress = 1;
      return;
    }
  if (len == 10 && !strncmp (option, "nocompress", len))
    {
      is_nocompress = 1;
      return;
    }
}

static void
parse_propeller_dis_options (const char *options)
{
  const char *option_end;

  if (options == NULL)
    return;
  while (*options != '\0') {
    /* Skip empty options */
    if (*options == ',')
      {
	options++;
	continue;
      }
      /* We know that *options is neither NUL or a comma.  */
      option_end = options + 1;
      while (*option_end != ',' && *option_end != '\0')
	option_end++;

      parse_prop_dis_option (options, option_end - options);

      /* Go on to the next one.  If option_end points to a comma, it
	 will be skipped above.  */
      options = option_end;
    }

}

static void
set_default_propeller_dis_options (struct disassemble_info *info)
{
  is_p2 = 0;
  is_compress = 0;
  is_nocompress = 0;

  /* check for p2 object files */
  if (info->flavour == bfd_target_elf_flavour && info->section != NULL)
    {
      Elf_Internal_Ehdr *header;

      header = elf_elfheader (info->section->owner);
      if ((header->e_flags & EF_PROPELLER_MACH) == EF_PROPELLER_PROP2)
	{
	  is_p2 = 1;
	}
    }
}


static int
is_propeller2(struct disassemble_info * info ATTRIBUTE_UNUSED)
{
  return is_p2;
}

static int
read_word (bfd_vma memaddr, int *word, struct disassemble_info * info)
{
  int status;
  bfd_byte x[4];

  status = (*info->read_memory_func) (memaddr, x, 4, info);
  if (status != 0)
    return -1;

  *word = x[3] << 24 | x[2] << 16 | x[1] << 8 | x[0];
  return 0;
}

static int
read_halfword (bfd_vma memaddr, int *word, disassemble_info * info)
{
  int status;
  bfd_byte x[4];

  status = (*info->read_memory_func) (memaddr, x, 2, info);
  if (status != 0)
    return -1;

  *word = x[1] << 8 | x[0];
  return 0;
}

static int
read_threebyte (bfd_vma memaddr, int *word, disassemble_info * info)
{
  int status;
  bfd_byte x[4];

  status = (*info->read_memory_func) (memaddr, x, 3, info);
  if (status != 0)
    return -1;

  *word = (x[2] << 16) | (x[1] << 8) | x[0];
  return 0;
}

static int
read_byte (bfd_vma memaddr, int *word, disassemble_info * info)
{
  int status;
  bfd_byte x[4];

  status = (*info->read_memory_func) (memaddr, x, 1, info);
  if (status != 0)
    return -1;

  *word = x[0];
  return 0;
}

/* Print the Propeller instruction at address MEMADDR in debugged memory,
   on INFO->STREAM.  Returns length of the instruction, in bytes.  */

#define AFTER_INSTRUCTION       "\t"
#define OPERAND_SEPARATOR       ", "
#define FPRINTF (*info->fprintf_func)
#define F       info->stream

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

static void
set_indirect_string(char *buf, int indcond, int regval)
{
  char reg = (regval & 1) ? 'B' : 'A';
  switch (indcond & 3) {
  case 0:
    sprintf (buf, "IND%c", reg);
    break;
  case 1:
    sprintf (buf, "IND%c++", reg);
    break;
  case 2:
    sprintf (buf, "IND%c--", reg);
    break;
  case 3:
    sprintf (buf, "++IND%c", reg);
    break;
  }
}

static int
print_insn_propeller32 (bfd_vma memaddr, struct disassemble_info *info, int opcode)
{
  int src, dst;
  int condition;
  int indcond;
  int set, immediate;
  int i;
  int need_zcr = 1;
  char *srcindirect, *dstindirect;
  char srcibuf[8], dstibuf[8];

  /* code for handling prop2 inda and indb operations */
  srcindirect = NULL;
  dstindirect = NULL;

  src = (opcode >> 0) & 0x1ff;
  dst = (opcode >> 9) & 0x1ff;
  condition = (opcode >> 18) & 0xf;
  immediate = (opcode >> 22) & 1;
  set = (opcode >> 23) & 7;

  indcond = condition;
  if (is_propeller2 (info)) {
    if ( (src == 0x1f6 || src == 0x1f7) && !immediate ) {
      srcindirect = srcibuf;
      set_indirect_string(srcibuf, indcond, src);
      condition = 15;
    }
    if (dst == 0x1f6 || dst == 0x1f7) {
      dstindirect = dstibuf;
      set_indirect_string(dstibuf, (indcond>>2), dst);
      condition = 15;
    }
  }
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
#define HWMATCH ((OP.hardware & (is_propeller2(info) ? PROP_2 : PROP_1)) != 0)
      if (HWMATCH && (opcode & OP.mask) == OP.opcode)
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
            else if (srcindirect)
	      {
		FPRINTF (F, "%s", srcindirect);
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
	      if (srcindirect)
		{
		  FPRINTF (F, "%s", srcindirect);
		}
	      else
		{
		  info->target = src<<2;
		  (*info->print_address_func) (info->target, info);
		}
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
	    if (dstindirect)
	      {
		  FPRINTF (F, "%s", dstindirect);
	      }
	    else
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
		if (srcindirect)
		  {
		    FPRINTF (F, "%s", srcindirect);
		  }
		else
		  {
		    info->target = src<<2;
		    (*info->print_address_func) (info->target, info);
		  }
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
          case PROPELLER_OPERAND_PTRS_OPS:
	    need_zcr = 0;
            FPRINTF (F, OP.name);
            FPRINTF (F, AFTER_INSTRUCTION);
	    if (dstindirect)
	      {
		FPRINTF (F, "%s", dstindirect);
	      }
	    else
              {
                info->target = dst<<2;
                (*info->print_address_func) (info->target, info);
              }
            FPRINTF (F, OPERAND_SEPARATOR);
            if (immediate)
              {
		int offset = src & 0x1f;
		int reg = (src & 0x100) ? 'B' : 'A';
		int update = (src & 0x80);
		int postmodify = (src & 0x40);

		if (src & 0x20) {
		  offset = -(32 - offset);
		}
		if (update) {
		  char *prestr = (offset < 0) ? "--" : "++";
		  char *poststr = (offset < 0) ? "--" : "++";
		  if (postmodify)
		    prestr = "";
		  else
		    poststr = "";
		  if (offset < 0)
		    offset = -offset;
		  FPRINTF (F, "%sPTR%c%s", prestr, reg, poststr);
		  if (offset != 1) {
		    FPRINTF (F, "[%d]", offset);
		  }
		} else {
		  FPRINTF (F, "PTR%c", reg);
		  if (offset != 0) {
		    FPRINTF (F, "[%d]", offset);
		  }
		}
              }
            else
              {
                info->target = src<<2;
                (*info->print_address_func) (info->target, info);
              }
	    goto done;
          case PROPELLER_OPERAND_PTRD_OPS:
          case PROPELLER_OPERAND_DESTIMM:
          case PROPELLER_OPERAND_DESTIMM_SRCIMM:
          case PROPELLER_OPERAND_SETINDA:
          case PROPELLER_OPERAND_SETINDB:
          case PROPELLER_OPERAND_SETINDS:
          case PROPELLER_OPERAND_REPD:
          case PROPELLER_OPERAND_REPS:
          case PROPELLER_OPERAND_JMPTASK:
          case PROPELLER_OPERAND_BIT:
            /* disassembly not implemented yet */
            FPRINTF (F, OP.name);
            goto done;
          default:
            /* TODO: is this a proper way of signalling an error? */
            FPRINTF (F, "<internal error: unrecognized instruction type: %d", OP.format);
            return -1;
          }
    }
done:
  if (i < propeller_num_opcodes)
    {
      if (need_zcr) {
	FPRINTF (F, " %s", flags[set + (OP.flags & FLAG_R_DEF ? 1 : 0) * 8]);
      }
    }
  else
    {
      FPRINTF (F, "<unrecognized instruction>");
    }

#undef OP
  return 4;
}

/*
 * try to figure out from the symbols whether this is compressed
 * or uncompressed code
 */

static int
is_compressed_code (bfd_vma pc, struct disassemble_info *info)
{
  int n, start;
  bfd_vma addr;
  elf_symbol_type *es;
  unsigned int type = 0;

  /* allow explicit option overrides */
  if (is_compress)
    return 1;
  if (is_nocompress)
    return 0;

  start = info->symtab_pos;
  for (n = start; n < info->symtab_size; n++)
    {
      if (n < 0) continue;
      addr = bfd_asymbol_value (info->symtab[n]);
      if (info->section != NULL && info->section != info->symtab[n]->section)
        continue; /* ignore symbol */
      if (addr > pc)
        break;
      es = *(elf_symbol_type **)(info->symtab + n);
      type = es->internal_elf_sym.st_other;
    }
  return (type & PROPELLER_OTHER_COMPRESSED) != 0;
}

static char *
xop_table[] = {
  "add\t%d, %s",
  "sub\t%d, %s",
  "cmps\t%d, %s wz, wc",
  "cmp\t%d, %s wz, wc",

  "and\t%d, %s",
  "andn\t%d, %s",
  "neg\t%d, %s",
  "or\t%d, %s",

  "xor\t%d, %s",
  "shl\t%d, %s",
  "shr\t%d, %s",
  "sar\t%d, %s",

  "rdbyte\t%d, %s",
  "rdlong\t%d, %s",
  "wrbyte\t%d, %s",
  "wrlong\t%d, %s"
};

static char *regname(int reg)
{
  static char tmpbuf[8];
  if (reg == 15)
    return "lr";
  sprintf(tmpbuf, "r%d", reg);
  return tmpbuf;
}

const char *macroname[] = {
  "nop",
  "break",
  "lret",
  "lpushm",

  "lpopm",
  "lpopret",
  "lcall",
  "mul",

  "udiv",
  "div",
  "mov",
  "xmov",

  "addsp",
  "???",
  "fcache"
  "native"
};

static void
print_opstring(struct disassemble_info *info,
               const char *str, int dest, int src, int imm)
{
  int c;
  while ( (c = *str++) != 0 )
    {
      if (c == '%') {
        c = *str++;
        if (!c) return;
        switch (c) {
        case 'd':
          FPRINTF ( F, "%s", regname(dest) );
          break;
        case 's':
          if (imm) {
            FPRINTF ( F, "#0x%x", src );
          } else {
            FPRINTF ( F, "%s", regname(src) );
          }
          break;
        case 'm':
          FPRINTF (F, "%s", macroname[dest & 0xf]);
          break;
        case 'a':
          if (imm) FPRINTF (F, "#");
          info->target = src;
          (*info->print_address_func) (info->target, info);
          break;
        case 'A':
          if (imm) FPRINTF (F, "#");
          info->target = src << 2;
          (*info->print_address_func) (info->target, info);
          break;
        default:
          FPRINTF ( F, "%c", c );
          break;
        }
      } else {
        FPRINTF ( F, "%c", c );
      }
    }
}

static int
print_macro (bfd_vma memaddr, struct disassemble_info *info, int which)
{
  int src, dst;
  int xmov_byte;
  int r = 0;
  bfd_vma newpc;

  switch (which)
    {
    case MACRO_NOP:
    case MACRO_BREAK:
    case MACRO_RET:
    case MACRO_MUL:
    case MACRO_DIV:
    case MACRO_UDIV:
      print_opstring (info, "\t\t%m", which, 0, 0);
      break;

    case MACRO_PUSHM:
    case MACRO_POPM:
    case MACRO_POPRET:
      if (read_byte (memaddr, &src, info) != 0)
        return -1;
      r = 1;
      print_opstring (info, "\t\t%m\t%s", which, src, 1);
      break;
    case MACRO_NATIVE:
      if (read_word (memaddr, &src, info) != 0)
        return -1;
      print_insn_propeller32 (memaddr, info, src);
      r = 4;
      break;
    case MACRO_XMVREG:
      if (read_byte (memaddr, &xmov_byte, info) != 0) return -1;
      src = xmov_byte & 0xf;
      dst = (xmov_byte >> 4) & 0xf;
      print_opstring (info, "\t\txmov\t%d,%s", dst, src, 0);
      if (read_byte (memaddr, &xmov_byte, info) != 0) return -1;
      src = xmov_byte & 0xf;
      dst = (xmov_byte >> 4) & 0xf;
      print_opstring (info, "\tmov\t%d,%s", dst, src, 0);
      r = 2;
      break;
    case MACRO_MVREG:
      if (read_byte (memaddr, &xmov_byte, info) != 0) return -1;
      src = xmov_byte & 0xf;
      dst = (xmov_byte >> 4) & 0xf;
      print_opstring (info, "\t\tmov\t%d,%s", dst, src, 0);
      r = 1;
      break;
    case MACRO_LCALL:
      if (read_halfword (memaddr, &src, info) != 0) return -1;
      print_opstring (info, "\t\tlcall\t%a", 0, src, 1);
      r = 2;
      break;
    case MACRO_FCACHE:
      if (read_halfword (memaddr, &src, info) != 0) return -1;
      print_opstring (info, "\t\tfcache\t%s", 0, src, 1);
      newpc = memaddr + 2;
      newpc = (memaddr + 3) & ~3;
      r = newpc - memaddr;
      break;
    case MACRO_ADDSP:
      if (read_byte (memaddr, &src, info) != 0) return -1;
      r = 1;
      if (src > 0x80) {
        src = 0x100 - src;
        print_opstring (info, "\t\tsub\tsp, %s", 0, src, 1);
      } else {
        print_opstring (info, "\t\tadd\tsp, %s", 0, src, 1);
      }
      break;
    default:
      FPRINTF (F, "\t\t???");
      break;
    }
  return r;
}

static int
do_compressed_insn (bfd_vma memaddr, struct disassemble_info *info)
{
  int opcode;
  int opprefix;
  int dst, src;
  int xmov_flag = 0;
  int xop;
  int xmov_byte;
  int imm;
  bfd_vma start_memaddr = memaddr;
  int r;

  if (read_byte (memaddr, &opcode, info) != 0)
    return -1;
  memaddr++;
  opprefix = opcode & 0xf0;
  dst = opcode & 0x0f;
  src = 0;

  switch (opprefix) {
  case PREFIX_MACRO:
    r = print_macro(memaddr, info, dst);
    if (r < 0) return r;
    memaddr += r;
    break;
  case PREFIX_XMOVREG:
  case PREFIX_XMOVIMM:
    xmov_flag = 1;
    if (read_byte (memaddr, &xmov_byte, info) != 0) return -1;
    src = xmov_byte & 0xf;
    dst = (xmov_byte >> 4) & 0xf;
    memaddr++;
    /* fall through */
  case PREFIX_REGREG:
  case PREFIX_REGIMM4:
  case PREFIX_REGIMM12:
    FPRINTF (F, "\t\t");
    if (xmov_flag)
      print_opstring(info, "xmov\t%d, %s\t", dst, src, 0);

    if (opprefix == PREFIX_REGIMM12) {
      if (read_halfword (memaddr, &src, info) != 0) return -1;
      memaddr += 2;
      xop = (src >> 12) & 0x0F;
      src = src & 0x0FFF;
    } else {
      if (read_byte (memaddr, &src, info) != 0) return -1;
      memaddr ++;
      xop = (src & 0x0F);
      src = (src >> 4) & 0x0F;
    }
    if (opprefix == PREFIX_XMOVREG || opprefix == PREFIX_REGREG)
      imm = 0;
    else
      imm = 1;
    if (src > 0x7FF) {
      src = src - 0x1000;
    }
    print_opstring (info, xop_table[xop], dst, src, imm);
    break;
  case PREFIX_MVI:
    FPRINTF (F, "\t\t");
    if (read_word (memaddr, &src, info) != 0) return -1;
    memaddr += 4;
    print_opstring (info, "mvi\t%d,%s", dst, src, 1);
    break;
  case PREFIX_MVIW:
    FPRINTF (F, "\t\t");
    if (read_halfword (memaddr, &src, info) != 0) return -1;
    memaddr += 2;
    print_opstring (info, "mviw\t%d,%s", dst, src, 1);
    break;
  case PREFIX_MVIB:
    FPRINTF (F, "\t\t");
    if (read_byte (memaddr, &src, info) != 0) return -1;
    memaddr += 1;
    print_opstring (info, "mov\t%d,%s", dst, src, 1);
    break;
  case PREFIX_ZEROREG:
    FPRINTF (F, "\t\t");
    print_opstring (info, "mov\t%d,%s", dst, 0, 1);
    break;
  case PREFIX_LEASP:
    FPRINTF (F, "\t\t");
    if (read_halfword (memaddr, &src, info) != 0) return -1;
    memaddr += 2;
    print_opstring (info, "leasp\t%d,%s", dst, src, 1);
    break;
  case PREFIX_PACK_NATIVE:
    if (read_threebyte (memaddr, &src, info) != 0) return -1;
    /* reconstruct the opcode */
    opcode = src & 0x3FFFF;
    xop = (src >> 18) & 0x3F;
    opcode = opcode | (xop << 26) | (0xF << 18) | (dst << 22);
    print_insn_propeller32(memaddr, info, opcode);
    memaddr += 3;
    break;
  case PREFIX_SKIP2:
  case PREFIX_SKIP3:
    FPRINTF (F, "\t%s\tskip", propeller_conditions[dst].name);
    break;
  case PREFIX_BRW:
    FPRINTF (F, "\t%s\t", propeller_conditions[dst].name);
    if (read_halfword (memaddr, &src, info) != 0) return -1;
    memaddr += 2;
    print_opstring (info, "brw\t%a", 0, src, 1);
    break;
  case PREFIX_BRS:
    FPRINTF (F, "\t%s\t", propeller_conditions[dst].name);
    if (read_byte (memaddr, &src, info) != 0) return -1;
    memaddr += 1;
    if (src > 0x80) src = src - 0x100;
    src += memaddr;
    print_opstring (info, "brs\t%a", 0, src, 1);
    break;
  default:
    FPRINTF (F, "\t\t<compressed>");
    break;
  }

  return memaddr - start_memaddr;
}

int
print_insn_propeller (bfd_vma memaddr, struct disassemble_info *info)
{
  int opcode;
  int r;
  int compress = 0;

  set_default_propeller_dis_options (info);
  parse_propeller_dis_options (info->disassembler_options);

  compress = is_compressed_code (memaddr, info);
  if (compress) {
    r = do_compressed_insn (memaddr, info);
  } else {

    if (read_word (memaddr, &opcode, info) != 0)
      return -1;
    r = print_insn_propeller32(memaddr, info, opcode);
  }
  if (r < 0) {
    memaddr += (compress ? 1 : 4);
    return r;
  }
  info->bytes_per_line = 4;
  info->bytes_per_chunk = compress ? 1 : 4;
  info->display_endian = BFD_ENDIAN_BIG;

  memaddr += r;
  return r;
}
