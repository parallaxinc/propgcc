/* tc-propeller
   Copyright 2011 Parallax Semiconductor, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to
   the Free Software Foundation, 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */

#include <stdio.h>
#include <stdlib.h>
#include "dis-asm.h"
#include "as.h"
#include "struc-symbol.h"
#include "safe-ctype.h"
#include "opcode/propeller.h"

/* A representation for Propeller machine code.  */
struct propeller_code
{
  char *error;
  int code;
  /* Is there an additional word?  */
  /* No real instructions need any, */
  /* but some fake ones might */
  int additional;
  int word;			/* Additional word, if any.  */
  struct
  {
    bfd_reloc_code_real_type type;
    expressionS exp;
    int pc_rel;
  } reloc;
};


/* These chars start a comment anywhere in a source file (except inside
   another comment.  */
const char comment_chars[] = "'";

/* These chars only start a comment at the beginning of a line.  */
const char line_comment_chars[] = "#/";

const char line_separator_chars[] = ";";

/* Chars that can be used to separate mant from exp in floating point nums.  */
const char EXP_CHARS[] = "eE";

/* Chars that mean this number is a floating point constant.  */
/* as in 0f123.456.  */
/* or    0H1.234E-12 (see exp chars above).  */
const char FLT_CHARS[] = "dDfF";

static void pseudo_fit (int);
static void pseudo_res (int);
static void pseudo_hub_ram (int);
static void pseudo_cog_ram (int);
static void pseudo_regbase (int);
static char *skip_whitespace (char *str);
static char *find_whitespace (char *str);
static char *find_whitespace_or_separator (char *str);

static int cog_ram = 1;		/* Use Cog ram by default */

static int register_base = 128;	/* Address of register file */

const pseudo_typeS md_pseudo_table[] = {
  {"fit", pseudo_fit, 0},
  {"res", pseudo_res, 0},
  {"hub_ram", pseudo_hub_ram, 0},
  {"cog_ram", pseudo_cog_ram, 0},
  {"regbase", pseudo_regbase, 0},
  {0, 0, 0},
};

static struct hash_control *insn_hash = NULL;
static struct hash_control *cond_hash = NULL;
static struct hash_control *eff_hash = NULL;

const char *md_shortopts = "m:";

struct option md_longopts[] = {
  {NULL, no_argument, NULL, 0}
};

size_t md_longopts_size = sizeof (md_longopts);

static void
init_defaults (void)
{
  static int first = 1;

  if (first)
    {
      /* set_option(as desired); */
      first = 0;
    }
}

void
md_begin (void)
{
  int i;

  init_defaults ();

  insn_hash = hash_new ();
  if (insn_hash == NULL)
    as_fatal (_("Virtual memory exhausted"));
  cond_hash = hash_new ();
  if (cond_hash == NULL)
    as_fatal (_("Virtual memory exhausted"));
  eff_hash = hash_new ();
  if (eff_hash == NULL)
    as_fatal (_("Virtual memory exhausted"));

  for (i = 0; i < propeller_num_opcodes; i++)
    hash_insert (insn_hash, propeller_opcodes[i].name,
		 (void *) (propeller_opcodes + i));
  for (i = 0; i < propeller_num_conditions; i++)
    hash_insert (cond_hash, propeller_conditions[i].name,
		 (void *) (propeller_conditions + i));
  for (i = 0; i < propeller_num_effects; i++)
    hash_insert (eff_hash, propeller_effects[i].name,
		 (void *) (propeller_effects + i));
}

long
md_chars_to_number (con, nbytes)
     unsigned char con[];	/* High order byte 1st.  */
     int nbytes;		/* Number of bytes in the input.  */
{
  switch (nbytes)
    {
    case 0:
      return 0;
    case 1:
      return con[3];
    case 2:
      return (con[2] << BITS_PER_CHAR) | con[3];
    case 4:
      return
	(((con[0] << BITS_PER_CHAR) | con[1]) << (2 * BITS_PER_CHAR))
	| ((con[2] << BITS_PER_CHAR) | con[3]);
    default:
      BAD_CASE (nbytes);
      return 0;
    }
}

/* Fix up some data or instructions after we find out the value of a symbol
   that they reference.  Knows about order of bytes in address.  */

void
md_apply_fix (fixS * fixP, valueT * valP, segT seg ATTRIBUTE_UNUSED)
{
  valueT code;
  valueT mask;
  valueT val = *valP;
  char *buf;
  int shift;
  int size;

  buf = fixP->fx_where + fixP->fx_frag->fr_literal;
  size = fixP->fx_size;
  code = md_chars_to_number ((unsigned char *) buf, size);

  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_PROPELLER_SRC:
      mask = 0x000001ff;
      shift = 0;
      break;
    case BFD_RELOC_PROPELLER_DST:
      mask = 0x0003fe00;
      shift = 9;
      break;
    case BFD_RELOC_32:
      mask = 0xffffffff;
      shift = 0;
      break;
    default:
      BAD_CASE (fixP->fx_r_type);
    }

  if (fixP->fx_addsy != NULL){
    val += symbol_get_bfdsym (fixP->fx_addsy)->section->vma;
  }
  code &= ~mask;
  code |= (val << shift) & mask;
  number_to_chars_bigendian (buf, code, size);

  if (fixP->fx_addsy == NULL && fixP->fx_pcrel == 0)
    fixP->fx_done = 1;
}

/* Translate internal representation of relocation info to BFD target
   format.  */

arelent *
tc_gen_reloc (asection * section ATTRIBUTE_UNUSED, fixS * fixp)
{
  arelent *reloc;
  bfd_reloc_code_real_type code;

  reloc = xmalloc (sizeof (*reloc));

  reloc->sym_ptr_ptr = xmalloc (sizeof (asymbol *));
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;

  /* This is taken account for in md_apply_fix().  */
  reloc->addend = -symbol_get_bfdsym (fixp->fx_addsy)->section->vma;

  switch (fixp->fx_r_type)
    {
    case BFD_RELOC_PROPELLER_SRC:
      code = BFD_RELOC_PROPELLER_SRC;
      break;

    case BFD_RELOC_PROPELLER_DST:
      code = BFD_RELOC_PROPELLER_DST;
      break;

    case BFD_RELOC_32:
      code = BFD_RELOC_32;
      break;

    default:
      BAD_CASE (fixp->fx_r_type);
      return NULL;
    }

  reloc->howto = bfd_reloc_type_lookup (stdoutput, code);

  if (reloc->howto == NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _
		    ("Can not represent %s relocation in this object file format"),
		    bfd_get_reloc_code_name (code));
      return NULL;
    }

  return reloc;
}

char *
md_atof (int type, char *litP, int *sizeP)
{
  (void) type;
  (void) litP;
  (void) sizeP;
  return 0;
}

/* Pseudo-op processing */
static void
pseudo_fit (int c ATTRIBUTE_UNUSED)
{
  /* Do nothing.  This is the linker's job */
}

static void
pseudo_res (int c ATTRIBUTE_UNUSED)
{
  int temp;

  temp = get_absolute_expression ();
  subseg_set (bss_section, temp);
  demand_empty_rest_of_line ();
}

static void
pseudo_hub_ram (int c ATTRIBUTE_UNUSED)
{
  cog_ram = 0;
}

static void
pseudo_cog_ram (int c ATTRIBUTE_UNUSED)
{
  cog_ram = 1;
}

static void
pseudo_regbase (int c ATTRIBUTE_UNUSED)
{
  register_base = get_absolute_expression ();
}

/* Instruction processing */
static char *
parse_expression (char *str, struct propeller_code *operand)
{
  char *save_input_line_pointer;
  char *end;
  char t;
  segT seg;

  str = skip_whitespace (str);
  end = find_whitespace_or_separator (str);
  t = *end;
  *end = 0;
  if (!strcasecmp ("lr", str))
    {
      operand->reloc.exp.X_add_number = 15 + register_base;
      operand->reloc.exp.X_op = O_register;
      *end = t;
      return end;
    }
  if (!strcasecmp ("sp", str))
    {
      operand->reloc.exp.X_add_number = 16 + register_base;
      operand->reloc.exp.X_op = O_register;
      *end = t;
      return end;
    }
  if (!strcasecmp ("pc", str))
    {
      operand->reloc.exp.X_add_number = 17 + register_base;
      operand->reloc.exp.X_op = O_register;
      *end = t;
      return end;
    }
  *end = t;
  if (*str == 'r' || *str == 'R')
    {
      int reg;
      reg = strtol (str + 1, &end, 10);
      if (end != str + 1)
	{
	  if (reg >= 0 && reg <= 15)
	    {
	      operand->reloc.exp.X_add_number = reg + register_base;
	      operand->reloc.exp.X_op = O_register;
	      return end;
	    }
	}
    }
  save_input_line_pointer = input_line_pointer;
  input_line_pointer = str;
  seg = expression (&operand->reloc.exp);
  if (seg == NULL)
    {
      input_line_pointer = save_input_line_pointer;
      operand->error = _("Error in expression");
      return str;
    }

  str = input_line_pointer;
  input_line_pointer = save_input_line_pointer;

  operand->reloc.pc_rel = 0;

  return str;
}

static char *
skip_whitespace (char *str)
{
  while (*str == ' ' || *str == '\t')
    str++;
  return str;
}

static char *
find_whitespace (char *str)
{
  while (*str != ' ' && *str != '\t' && *str != 0)
    str++;
  return str;
}

static char *
find_whitespace_or_separator (char *str)
{
  while (*str != ' ' && *str != '\t' && *str != 0 && *str != ',')
    str++;
  return str;
}

static char *
parse_separator (char *str, int *error)
{
  str = skip_whitespace (str);
  *error = (*str != ',');
  if (!*error)
    str++;
  return str;
}

static void
lc (char *str)
{
  while (*str)
    {
      *str = TOLOWER (*str);
      str++;
    }
}

void
md_assemble (char *instruction_string)
{
  const struct propeller_opcode *op;
  const struct propeller_condition *cond;
  const struct propeller_effect *eff;
  struct propeller_code insn, op1, op2;
  int error;
  int size;
  char *err = NULL;
  char *str;
  char *p;
  char c;

  str = skip_whitespace (instruction_string);
  p = find_whitespace (str);
  if (p - str == 0)
    {
      as_bad (_("No instruction found"));
      return;
    }

  c = *p;
  *p = '\0';
  lc (str);
  cond = (struct propeller_condition *) hash_find (cond_hash, str);
  *p = c;
  if (cond)
    {
      char *p2;
      /* Process conditional flag that str points to */
      insn.code = cond->value;
      p = skip_whitespace (p);
      p2 = find_whitespace (p);
      if (p2 - p == 0)
	{
	  as_bad (_("No instruction found after condition"));
	  return;
	}
      str = p;
      p = p2;
    }
  else
    {
      insn.code = 0xf << 18;
    }
  c = *p;
  *p = '\0';
  lc (str);
  op = (struct propeller_opcode *) hash_find (insn_hash, str);
  *p = c;
  if (op == 0)
    {
      as_bad (_("Unknown instruction '%s'"), str);
      return;
    }

  insn.error = NULL;
  insn.code |= op->opcode;
  insn.reloc.type = BFD_RELOC_NONE;
  op1.error = NULL;
  op1.additional = FALSE;
  op1.reloc.type = BFD_RELOC_NONE;
  op2.error = NULL;
  op2.additional = FALSE;
  op2.reloc.type = BFD_RELOC_NONE;

  str = p;
  size = 4;

  switch (op->format)
    {
    case PROPELLER_OPERAND_IGNORE:
      /* special case for NOP instruction, since we need to 
       * suppress the condition. */
      insn.code = 0;
      break;
    case PROPELLER_OPERAND_NO_OPS:
      str = skip_whitespace (str);
      if (*str == 0)
	str = "";
      break;

    case PROPELLER_OPERAND_TWO_OPS:
    case PROPELLER_OPERAND_DEST_ONLY:
      str = skip_whitespace (str);
      str = parse_expression (str, &op1);
      if (op1.error)
	break;
      switch (op1.reloc.exp.X_op)
	{
	case O_constant:
	case O_register:
	  if (op1.reloc.exp.X_add_number & ~0x1ff)
	    {
	      op1.error = _("9-bit value out of range");
	      break;
	    }
	  insn.code |= op1.reloc.exp.X_add_number << 9;
	  break;
	case O_symbol:
	case O_add:
	case O_subtract:
	  op1.reloc.type = BFD_RELOC_PROPELLER_DST;
	  op1.reloc.pc_rel = 0;
	  break;
	case O_illegal:
	  op1.error = _("Illegal operand in operand 1");
	  break;
	default:
	  op1.error = _("Unhandled case in op1");
	}
      if (op->format == PROPELLER_OPERAND_DEST_ONLY)
	{
	  break;
	}
      else
	{
	  str = parse_separator (str, &error);
	  if (error)
	    {
	      op2.error = _("Missing ','");
	      break;
	    }
	}
      /* Fall through */
    case PROPELLER_OPERAND_SOURCE_ONLY:
      str = skip_whitespace (str);
      if (*str == '#')
	{
	  str++;
	  insn.code |= 1 << 22;
	}
      str = parse_expression (str, &op2);
      if (op2.error)
	break;
      switch (op2.reloc.exp.X_op)
	{
	case O_constant:
	case O_register:
	  if (op2.reloc.exp.X_add_number & ~0x1ff)
	    {
	      op2.error = _("9-bit value out of range");
	      break;
	    }
	  insn.code |= op2.reloc.exp.X_add_number;
	  break;
	case O_symbol:
	case O_add:
	case O_subtract:
	  op2.reloc.type = BFD_RELOC_PROPELLER_SRC;
	  op2.reloc.pc_rel = 0;
	  break;
	case O_illegal:
	  op2.error = _("Illegal operand in operand 2");
	  break;
	default:
	  op2.error = _("Unhandled case in op2");
	}
      break;
    case PROPELLER_OPERAND_CALL:
      {
	char *str2 = malloc (5 + strlen (str));
	if (str2 == NULL)
	  as_fatal (_("Virtual memory exhausted"));
	strcpy (str2, str);
	str = parse_expression (str, &op2);
	if (op2.error)
	  break;
	switch (op2.reloc.exp.X_op)
	  {
	  case O_constant:
	  case O_register:
	    if (op2.reloc.exp.X_add_number & ~0x1ff)
	      {
		op2.error = _("9-bit value out of range");
		break;
	      }
	    insn.code |= op2.reloc.exp.X_add_number;
	    break;
	  case O_symbol:
	  case O_add:
	  case O_subtract:
	    op2.reloc.type = BFD_RELOC_PROPELLER_SRC;
	    op2.reloc.pc_rel = 0;
	    break;
	  case O_illegal:
	    op1.error = _("Illegal operand in call");
	    break;
	  default:
	    op2.error = _("Unhandled case in call");
	  }
	strcat (str2, "_ret");
	parse_expression (str2, &op1);
	free (str2);
	if (op1.error)
	  break;
	switch (op1.reloc.exp.X_op)
	  {
	  case O_symbol:
	    op1.reloc.type = BFD_RELOC_PROPELLER_DST;
	    op1.reloc.pc_rel = 0;
	    break;
	  default:
	    op1.error = _("Improper call target");
	  }
      }
      break;
    default:
      BAD_CASE (op->format);
    }

  /* set the r bit to its default state for this insn */
  insn.code |= op->result << 23;

  /* Find and process any effect flags */
  do
    {
      str = skip_whitespace (str);
      p = find_whitespace_or_separator (str);
      c = *p;
      *p = '\0';
      lc (str);
      eff = (struct propeller_effect *) hash_find (eff_hash, str);
      *p = c;
      if (!eff)
	break;
      str = p;
      insn.code |= eff->or;
      insn.code &= eff->and;
      str = parse_separator (str, &error);
    }
  while (eff && !error);

  if (op1.error)
    err = op1.error;
  else if (op2.error)
    err = op2.error;
  else
    {
      str = skip_whitespace (str);
      if (*str)
	err = _("Too many operands");
    }

  if (err)
    {
      as_bad ("%s", err);
      return;
    }
  {
    char *to = NULL;

    if (err)
      {
	as_bad ("%s", err);
	return;
      }

    to = frag_more (size);

    md_number_to_chars (to, insn.code, 4);
    if (insn.reloc.type != BFD_RELOC_NONE)
      fix_new_exp (frag_now, to - frag_now->fr_literal, 4,
		   &insn.reloc.exp, insn.reloc.pc_rel, insn.reloc.type);
    if (op1.reloc.type != BFD_RELOC_NONE)
      fix_new_exp (frag_now, to - frag_now->fr_literal, 4,
		   &op1.reloc.exp, op1.reloc.pc_rel, op1.reloc.type);
    if (op2.reloc.type != BFD_RELOC_NONE)
      fix_new_exp (frag_now, to - frag_now->fr_literal, 4,
		   &op2.reloc.exp, op2.reloc.pc_rel, op2.reloc.type);
    to += 4;

    /* These are never used for real instructions, but might be useful */
    /* for some pseudoinstruction for LMM or such. */
    if (op1.additional)
      {
	md_number_to_chars (to, op1.word, 4);
	if (op1.reloc.type != BFD_RELOC_NONE)
	  fix_new_exp (frag_now, to - frag_now->fr_literal, 4,
		       &op1.reloc.exp, op1.reloc.pc_rel, op1.reloc.type);
	to += 4;
      }

    if (op2.additional)
      {
	md_number_to_chars (to, op2.word, 4);
	if (op2.reloc.type != BFD_RELOC_NONE)
	  fix_new_exp (frag_now, to - frag_now->fr_literal, 4,
		       &op2.reloc.exp, op2.reloc.pc_rel, op2.reloc.type);
      }
  }
}

int
md_estimate_size_before_relax (fragS * fragP ATTRIBUTE_UNUSED,
			       segT segment ATTRIBUTE_UNUSED)
{
  return 0;
}

void
md_convert_frag (bfd * headers ATTRIBUTE_UNUSED,
		 segT seg ATTRIBUTE_UNUSED, fragS * fragP ATTRIBUTE_UNUSED)
{
}

void
propeller_frob_label (symbolS * sym)
{
  sym->sy_tc = cog_ram;
}

void
propeller_frob_symbol (symbolS * sym, int punt ATTRIBUTE_UNUSED)
{
  if (sym->sy_tc)
    {
        sym->sy_value.X_add_number /= 4;
    }
}

int md_short_jump_size = 4;
int md_long_jump_size = 4;

void
md_create_short_jump (char *ptr ATTRIBUTE_UNUSED,
		      addressT from_addr ATTRIBUTE_UNUSED,
		      addressT to_addr ATTRIBUTE_UNUSED,
		      fragS * frag ATTRIBUTE_UNUSED,
		      symbolS * to_symbol ATTRIBUTE_UNUSED)
{
}

void
md_create_long_jump (char *ptr ATTRIBUTE_UNUSED,
		     addressT from_addr ATTRIBUTE_UNUSED,
		     addressT to_addr ATTRIBUTE_UNUSED,
		     fragS * frag ATTRIBUTE_UNUSED,
		     symbolS * to_symbol ATTRIBUTE_UNUSED)
{
}


/* Invocation line includes a switch not recognized by the base assembler.
   See if it's a processor-specific option.  */

int
md_parse_option (int c, char *arg)
{
  (void) c;
  (void) arg;
  return 0;
}

void
md_show_usage (FILE * stream)
{
  fprintf (stream, "\
Propeller options\n\
\tNone at this time.\n\
");
}

symbolS *
md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
  return 0;
}

valueT
md_section_align (segT segment ATTRIBUTE_UNUSED, valueT size)
{
  return (size + 3) & ~3;
}

long
md_pcrel_from (fixS * fixP)
{
  return fixP->fx_frag->fr_address + fixP->fx_where + fixP->fx_size;
}
