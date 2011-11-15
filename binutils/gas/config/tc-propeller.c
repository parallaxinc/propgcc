/* tc-propeller
   Copyright 2011 Parallax Inc.

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
#include "elf/propeller.h"

/* A representation for Propeller machine code.  */
struct propeller_code
{
  char *error;
  int code;
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
static char *skip_whitespace (char *str);
static char *find_whitespace (char *str);
static char *find_whitespace_or_separator (char *str);

static int cog_ram = 0;		/* Use Cog ram if 1 */
static int lmm = 0;             /* Enable LMM pseudo-instructions */

const pseudo_typeS md_pseudo_table[] = {
  {"fit", pseudo_fit, 0},
  {"res", pseudo_res, 0},
  {"hub_ram", pseudo_hub_ram, 0},
  {"cog_ram", pseudo_cog_ram, 0},
  {0, 0, 0},
};

static struct hash_control *insn_hash = NULL;
static struct hash_control *cond_hash = NULL;
static struct hash_control *eff_hash = NULL;

const char *md_shortopts = "";

struct option md_longopts[] = {
  {"lmm", no_argument, NULL, OPTION_MD_BASE},
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

  for (i = 0; i < propeller_num_opcodes; i++){
    switch(propeller_opcodes[i].hardware){
    case PROP_1:
      break;
    case PROP_1_LMM:
      if(lmm) break;
      continue;
    default:
      continue;
    }
    hash_insert (insn_hash, propeller_opcodes[i].name,
		 (void *) (propeller_opcodes + i));
  }
  for (i = 0; i < propeller_num_conditions; i++)
    hash_insert (cond_hash, propeller_conditions[i].name,
		 (void *) (propeller_conditions + i));
  for (i = 0; i < propeller_num_effects; i++)
    hash_insert (eff_hash, propeller_effects[i].name,
		 (void *) (propeller_effects + i));

  /* make sure data and bss are longword aligned */
  record_alignment(data_section, 2);
  record_alignment(bss_section, 2);
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
      return con[0];
    case 2:
      return (con[1] << BITS_PER_CHAR) | con[0];
    case 4:
      return
	(((con[3] << BITS_PER_CHAR) | con[2]) << (2 * BITS_PER_CHAR))
	| ((con[1] << BITS_PER_CHAR) | con[0]);
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
  int rshift;
  int size;

  buf = fixP->fx_where + fixP->fx_frag->fr_literal;
  size = fixP->fx_size;
  code = md_chars_to_number ((unsigned char *) buf, size);

  /* On a 64-bit host, silently truncate 'value' to 32 bits for
     consistency with the behaviour on 32-bit hosts.  Remember value
     for emit_reloc.  */
  val &= 0xffffffff;
  val ^= 0x80000000;
  val -= 0x80000000;

  *valP = val;
  fixP->fx_addnumber = val;

  /* Same treatment for fixP->fx_offset.  */
  fixP->fx_offset &= 0xffffffff;
  fixP->fx_offset ^= 0x80000000;
  fixP->fx_offset -= 0x80000000;

  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_PROPELLER_SRC_IMM:
      mask = 0x000001ff;
      shift = 0;
      rshift = 0;
      break;
    case BFD_RELOC_PROPELLER_SRC:
      mask = 0x000001ff;
      shift = 0;
      rshift = 2;
      break;
    case BFD_RELOC_PROPELLER_DST:
      mask = 0x0003fe00;
      shift = 9;
      rshift = 2;
      break;
    case BFD_RELOC_PROPELLER_23:
      mask = 0x007fffff;
      shift = 0;
      rshift = 0;
      break;
    case BFD_RELOC_32:
      mask = 0xffffffff;
      shift = 0;
      rshift = 0;
      break;
    case BFD_RELOC_16:
      mask = 0x0000ffff;
      shift = 0;
      rshift = 0;
      break;
    case BFD_RELOC_8:
      mask = 0x000000ff;
      shift = 0;
      rshift = 0;
      break;
    case BFD_RELOC_PROPELLER_PCREL10:
      mask = 0x000001ff;
      shift = 0;
      rshift = 0;
      if ((val & 0x80000000)) {
	/* negative */
	//fprintf(stderr, "negative val=(%08lx)\n", val);
	val = (-val) & 0xffffffff;
	val |=  0x04000000;  /* toggle add to sub */
	mask |= 0x04000000;
      }
      break;
    default:
      BAD_CASE (fixP->fx_r_type);
    }

  if (fixP->fx_addsy != NULL){
    val += symbol_get_bfdsym (fixP->fx_addsy)->section->vma;
  } else if (fixP->fx_subsy != NULL) {
    val -= symbol_get_bfdsym (fixP->fx_subsy)->section->vma;
  }

  if( (((val>>rshift) << shift) & 0xffffffff) & ~mask){
    as_bad_where (fixP->fx_file, fixP->fx_line,
		  _("Relocation overflows"));
    //fprintf(stderr, "val=(%08lx), mask=%08lx, shift=%d, rshift=%d\n",
    //	    (unsigned long)val, (unsigned long)mask, shift, rshift);
  }

  {
    code &= ~mask;
    code |= ((val>>rshift) << shift) & mask;
  }
  md_number_to_chars (buf, code, size);

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

  reloc->addend = fixp->fx_offset;

  switch (fixp->fx_r_type)
    {
    case BFD_RELOC_32:
    case BFD_RELOC_16:
    case BFD_RELOC_8:
    case BFD_RELOC_PROPELLER_SRC:
    case BFD_RELOC_PROPELLER_SRC_IMM:
    case BFD_RELOC_PROPELLER_DST:
    case BFD_RELOC_PROPELLER_23:
    case BFD_RELOC_PROPELLER_PCREL10:
      code = fixp->fx_r_type;
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
  /* does nothing interesting right now, but
     we do parse the expression
  */
  get_absolute_expression ();
  demand_empty_rest_of_line ();
}

static void
pseudo_res (int c ATTRIBUTE_UNUSED)
{
  s_space(4);
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

/* Instruction processing */
static char *
parse_expression (char *str, struct propeller_code *operand)
{
  char *save_input_line_pointer;
  segT seg;

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

static char *
parse_src(char *str, struct propeller_code *operand, struct propeller_code *insn, int format){
  int integer_reloc = 0;
  int pcrel_reloc = 0;

  str = skip_whitespace (str);
  if (*str == '#')
    {
      str++;
      insn->code |= 1 << 22;
      if (format != PROPELLER_OPERAND_JMP && format != PROPELLER_OPERAND_JMPRET && format != PROPELLER_OPERAND_MOVA)
	{
	  integer_reloc = 1;
	}
    }

  if (format == PROPELLER_OPERAND_BRS)
    pcrel_reloc = 1;

  str = parse_expression (str, operand);
  if (operand->error)
    return str;
  switch (operand->reloc.exp.X_op)
    {
    case O_constant:
    case O_register:
      if (operand->reloc.exp.X_add_number & ~0x1ff)
	{
	  operand->error = _("9-bit value out of range");
	  break;
	}
      insn->code |= operand->reloc.exp.X_add_number;
      break;
    case O_symbol:
    case O_add:
    case O_subtract:
      if (pcrel_reloc)
	{
	  operand->reloc.type = BFD_RELOC_PROPELLER_PCREL10;
	  operand->reloc.pc_rel = 1;
	}
      else
	{
	  operand->reloc.type = integer_reloc ? BFD_RELOC_PROPELLER_SRC_IMM : BFD_RELOC_PROPELLER_SRC;
	  operand->reloc.pc_rel = 0;
	}
      break;
    case O_illegal:
      operand->error = _("Illegal operand in source");
      break;
    default:
      if (cog_ram)
	operand->error = _("Source operand too complicated for .cog_ram");
      else if (pcrel_reloc)
	operand->error = _("Source operand too complicated for brs instruction");
      else
	{
	  operand->reloc.type = integer_reloc ? BFD_RELOC_PROPELLER_SRC_IMM : BFD_RELOC_PROPELLER_SRC;
	  operand->reloc.pc_rel = 0;
	}
      break;
    }
  return str;
}

static char *
parse_src_n(char *str, struct propeller_code *operand, int nbits){

  str = skip_whitespace (str);
  if (*str++ != '#')
    {
      operand->error = _("immediate operand required");
      return str;
    }

  str = parse_expression (str, operand);
  if (operand->error)
    return str;
  switch (operand->reloc.exp.X_op)
    {
    case O_constant:
    case O_register:
      if (operand->reloc.exp.X_add_number & ((1 << nbits)-1))
	{
	  operand->error = _("value out of range");
	  break;
	}
      operand->code = operand->reloc.exp.X_add_number;
      break;
    case O_symbol:
    case O_add:
    case O_subtract:
      operand->reloc.type = BFD_RELOC_PROPELLER_23;
      operand->reloc.pc_rel = 0;
      break;
    case O_illegal:
      operand->error = _("Illegal operand in source");
      break;
    default:
      if (cog_ram)
	operand->error = _("Source operand too complicated for .cog_ram");
      else
	{
	  operand->reloc.type = BFD_RELOC_PROPELLER_23;
	  operand->reloc.pc_rel = 0;
	}
      break;
    }
  return str;
}

static char *
parse_dest(char *str, struct propeller_code *operand, struct propeller_code *insn){
  str = skip_whitespace (str);
  str = parse_expression (str, operand);
  if (operand->error)
    return str;
  switch (operand->reloc.exp.X_op)
    {
    case O_constant:
    case O_register:
      if (operand->reloc.exp.X_add_number & ~0x1ff)
	{
	  operand->error = _("9-bit value out of range");
	  break;
	}
      insn->code |= operand->reloc.exp.X_add_number << 9;
      break;
    case O_symbol:
    case O_add:
    case O_subtract:
      operand->reloc.type = BFD_RELOC_PROPELLER_DST;
      operand->reloc.pc_rel = 0;
      break;
    case O_illegal:
      operand->error = _("Illegal operand in destination");
      break;
    default:
      if (cog_ram)
	operand->error = _("Destination operand in .cog_ram too complicated");
      else
	{
	  operand->reloc.type = BFD_RELOC_PROPELLER_DST;
	  operand->reloc.pc_rel = 0;
	}
      break;
    }
  return str;
}

void
md_assemble (char *instruction_string)
{
  const struct propeller_opcode *op;
  const struct propeller_condition *cond;
  const struct propeller_effect *eff;
  struct propeller_code insn, op1, op2, op3;
  int error;
  int size;
  char *err = NULL;
  char *str;
  char *p;
  char c;

  /* force 4 byte alignment for this section */
  record_alignment(now_seg, 2);

  /* remove carriage returns (convert them to spaces) in case we are
     in dos mode */
  for (p = instruction_string; *p; p++)
    if (*p == '\r') *p = ' ';

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
  op1.reloc.type = BFD_RELOC_NONE;
  op2.error = NULL;
  op2.reloc.type = BFD_RELOC_NONE;
  op3.error = NULL;
  op3.reloc.type = BFD_RELOC_NONE;

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

    case PROPELLER_OPERAND_DEST_ONLY:
      str = parse_dest(str, &op1, &insn);
      break;

    case PROPELLER_OPERAND_TWO_OPS:
    case PROPELLER_OPERAND_JMPRET:
    case PROPELLER_OPERAND_MOVA:
      str = parse_dest(str, &op1, &insn);
      str = parse_separator (str, &error);
      if (error)
	{
	  op2.error = _("Missing ','");
	  break;
	}
      str = parse_src(str, &op2, &insn, op->format);
      break;

    case PROPELLER_OPERAND_LDI:
      {
	char *pc;
	str = parse_dest(str, &op1, &insn);
	str = parse_separator (str, &error);
	if (error)
	  {
	    op3.error = _("Missing ','");
	    break;
	  }
	pc = malloc(3);
	if (pc == NULL)
	  as_fatal (_("Virtual memory exhausted"));
  	strcpy (pc, "pc");
	parse_src(pc, &op2, &insn, PROPELLER_OPERAND_TWO_OPS);
	str = parse_src_n(str, &op3, 32);
	size = 8;
	if(op3.reloc.exp.X_op == O_constant){
	  /* Be sure to adjust this as needed for Prop-2! FIXME */
	  if((op3.reloc.exp.X_add_number & 0x003c0000) && (op3.reloc.exp.X_add_number & 0x03800000))
	    {
	      op3.error = _("value out of range");
	      break;
	    }
	  op3.code = op3.reloc.exp.X_add_number;
	}
	free(pc);
      }
      break;

    case PROPELLER_OPERAND_BRS:
      {
	char *arg;
	char *arg2;
	int len;
	len = strlen(str);
	arg = malloc(len+16);
	if (arg == NULL)
	  as_fatal (_("Virtual memory exhausted"));
	sprintf(arg, "pc,#%s", str);
	str += len;
	arg2 = parse_dest (arg, &op1, &insn);
	arg2 = parse_separator (arg2, &error);
        if (error)
	  {
	   op2.error = _("Missing ','");
	   break;
	  }
        arg2 = parse_src (arg2, &op2, &insn, op->format);
	free (arg);
      }
      break;
    case PROPELLER_OPERAND_BRL:
      {
        char *arg;
	char *arg2;
        int len = strlen(str);
	arg = malloc(len+16);
	if (arg == NULL)
	  as_fatal (_("Virtual memory exhausted"));
	sprintf(arg, "pc,pc,#%s", str);
	str += len;
        arg2 = parse_dest(arg, &op1, &insn);
        arg2 = parse_separator (arg2, &error);
        if (error)
	  {
	   op2.error = _("Missing ','");
	   break;
	  }
        arg2 = parse_src(arg2, &op2, &insn, op->format);
        arg2 = parse_separator (arg2, &error);
        if (error)
	  {
	   op3.error = _("Missing ','");
	   break;
	  }
	arg2 = parse_src_n(arg2, &op3, 23);
	size = 8;
	free(arg);
      }
      break;

    case PROPELLER_OPERAND_SOURCE_ONLY:
    case PROPELLER_OPERAND_JMP:
      str = parse_src(str, &op2, &insn, op->format);
      break;

    case PROPELLER_OPERAND_CALL:
      {
	char *str2, *p2;
        str = skip_whitespace (str);
        if (*str == '#')
	  {
	    str++;
	    insn.code |= 1 << 22;
	  }
	str2 = malloc (5 + strlen (str));
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
	  case O_illegal:
	    op1.error = _("Illegal operand in call");
	    break;
	  default:
	    op2.reloc.type = BFD_RELOC_PROPELLER_SRC;
	    op2.reloc.pc_rel = 0;
	    break;
	  }

	p2 = find_whitespace_or_separator (str2);
	*p2 = 0;
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
      if(*str == 0) break;
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
  else if (op3.error)
    err = op3.error;
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

    /* op3 is never used for real instructions, but is useful */
    /* for some pseudoinstruction for LMM and such. */
    if (op3.reloc.type != BFD_RELOC_NONE || op3.code)
      {
	md_number_to_chars (to, op3.code, 4);
	if(op3.reloc.type != BFD_RELOC_NONE){
	  fix_new_exp (frag_now, to - frag_now->fr_literal, 4,
		       &op3.reloc.exp, op3.reloc.pc_rel, op3.reloc.type);
	}
	to += 4;
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
  symbol_set_tc (sym, &cog_ram);
}

void
propeller_frob_symbol (symbolS * sym, int punt ATTRIBUTE_UNUSED)
{
  int *sy_tc = symbol_get_tc (sym);
  if (*sy_tc)
    {
      S_SET_OTHER (sym, PROPELLER_OTHER_COG_RAM
		   | (S_GET_OTHER (sym) & ~PROPELLER_OTHER_FLAGS));
    }
}

valueT
propeller_s_get_value (symbolS *s)
{
  valueT val = S_GET_VALUE(s);
  int *sy_tc = symbol_get_tc (s);
  if(*sy_tc){
    val /= 4;
  }
  return val;
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
  (void)arg;
  switch (c)
    {
    case OPTION_MD_BASE:
      lmm = 1;
      break;
    default:
      return 0;
    }
  return 1;
}

void
md_show_usage (FILE * stream)
{
  fprintf (stream, "\
Propeller options\n\
  --lmm\t\tEnable LMM instructions.\n\
");
}

symbolS *
md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
  return 0;
}

/*
 * round up a section's size to the appropriate boundary
 */
valueT
md_section_align (segT segment, valueT size)
{
  int align = bfd_get_section_alignment (stdoutput, segment);
  valueT mask = ((valueT) 1 << align) - 1;

  return (size + mask) & ~mask;
}

long
md_pcrel_from (fixS * fixP)
{
  return fixP->fx_frag->fr_address + fixP->fx_where + fixP->fx_size;
}
