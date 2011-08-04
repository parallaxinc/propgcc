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
#include "dis-asm.h"
#include "as.h"
#include "safe-ctype.h"
#include "opcode/propeller.h"

/* A representation for Propeller machine code.  */
struct propeller_code
{
  char *error;
  int code;
  int additional;	/* Is there an additional word?  */
  int word;		/* Additional word, if any.  */
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

const pseudo_typeS md_pseudo_table[] =
{
  { 0, 0, 0 },
};

static struct hash_control *insn_hash = NULL;

const char *md_shortopts = "m:";

struct option md_longopts[] =
{
  { NULL, no_argument, NULL, 0 }
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

  for (i = 0; i < propeller_num_opcodes; i++)
    hash_insert (insn_hash, propeller_opcodes[i].name, (void *) (propeller_opcodes + i));
}

void
md_number_to_chars (char con[], valueT value, int nbytes)
{
  switch (nbytes)
    {
    case 0:
      break;
    case 1:
      con[0] =  value       & 0xff;
      break;
    case 2:
      con[0] =  value        & 0xff;
      con[1] = (value >>  8) & 0xff;
      break;
    case 4:
      con[3] =  value        & 0xff;
      con[2] = (value >>  8) & 0xff;
      con[1] = (value >> 16) & 0xff;
      con[0] = (value >> 24) & 0xff;
      break;
    default:
      BAD_CASE (nbytes);
    }
}


/* Fix up some data or instructions after we find out the value of a symbol
   that they reference.  Knows about order of bytes in address.  */

void
md_apply_fix (fixS *fixP,
	       valueT * valP,
	       segT seg ATTRIBUTE_UNUSED)
{
  (void)fixP;
  (void)valP;
}

long
md_chars_to_number (con, nbytes)
     unsigned char con[];	/* Low order byte 1st. FIXME  */
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
	(((con[0] << BITS_PER_CHAR) | con[1]) << (2 * BITS_PER_CHAR))
	|((con[2] << BITS_PER_CHAR) | con[3]);
    default:
      BAD_CASE (nbytes);
      return 0;
    }
}

char *
md_atof (int type, char * litP, int * sizeP)
{
  (void)type;
  (void)litP;
  (void)sizeP;
  return 0;
}

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
parse_separator (char *str, int *error)
{
  str = skip_whitespace (str);
  *error = (*str != ',');
  if (!*error)
    str++;
  return str;
}

void
md_assemble (char *instruction_string)
{
  const struct propeller_opcode *op;
  struct propeller_code insn, op1, op2;
  int error;
  int size;
  int op_is_immediate = 0;
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

  if(!strncasecmp("if_", str, 3)){
    char *p2;
    /* Process conditional flag that str points to */
    p = skip_whitespace(p);
    p2 = find_whitespace(p);
    if(p2 - p == 0){
      as_bad (_("No instruction found after condition"));
      return;
    }
    str = p;
    p = p2;
  }
  c = *p;
  *p = '\0';
  op = (struct propeller_opcode *)hash_find (insn_hash, str);
  *p = c;
  if (op == 0)
    {
      as_bad (_("Unknown instruction '%s'"), str);
      return;
    }

  insn.error = NULL;
  insn.code = op->opcode;
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
    case PROPELLER_OPERAND_NO_OPS:
      str = skip_whitespace (str);
      if (*str == 0)
	str = "";
      break;

    case PROPELLER_OPERAND_TWO_OPS:
    case PROPELLER_OPERAND_SOURCE_ONLY:
      str = skip_whitespace (str);
      if (*str == '#'){
	str++;
	op_is_immediate = 1;
      }
      str = parse_expression (str, &op1);
      if (op1.error)
	break;
      if (op1.reloc.exp.X_op != O_constant || op1.reloc.type != BFD_RELOC_NONE)
	{
	  op1.error = _("operand is not an absolute constant");
	  break;
	}
      if (op1.reloc.exp.X_add_number & ~0x1ff)
        {
          op1.error = _("9-bit value out of range");
          break;
        }
      insn.code |= op1.reloc.exp.X_add_number;
      if(op->format == PROPELLER_OPERAND_SOURCE_ONLY){
        break;
      } else {
        str = parse_separator (str, &error);
        if (error)
	  {
	    op2.error = _("Missing ','");
	    break;
	  }
	}
    case PROPELLER_OPERAND_DEST_ONLY:
      str = parse_expression (str, &op2);
      if (op2.error)
	break;
      if (op2.reloc.exp.X_op != O_constant || op2.reloc.type != BFD_RELOC_NONE)
	{
	  op2.error = _("operand is not an absolute constant");
	  break;
	}
      if (op2.reloc.exp.X_add_number & ~0x1ff)
        {
          op2.error = _("9-bit value out of range");
          break;
        }
      insn.code |= op2.reloc.exp.X_add_number;
      break;
    default:
      BAD_CASE (op->format);
    }

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
}

int
md_estimate_size_before_relax (fragS *fragP ATTRIBUTE_UNUSED,
			       segT segment ATTRIBUTE_UNUSED)
{
  return 0;
}

void
md_convert_frag (bfd *headers ATTRIBUTE_UNUSED,
		 segT seg ATTRIBUTE_UNUSED,
		 fragS *fragP ATTRIBUTE_UNUSED)
{
}

int md_short_jump_size = 4;
int md_long_jump_size = 4;

void
md_create_short_jump (char *ptr ATTRIBUTE_UNUSED,
		      addressT from_addr ATTRIBUTE_UNUSED,
		      addressT to_addr ATTRIBUTE_UNUSED,
		      fragS *frag ATTRIBUTE_UNUSED,
		      symbolS *to_symbol ATTRIBUTE_UNUSED)
{
}

void
md_create_long_jump (char *ptr ATTRIBUTE_UNUSED,
		     addressT from_addr ATTRIBUTE_UNUSED,
		     addressT to_addr ATTRIBUTE_UNUSED,
		     fragS *frag ATTRIBUTE_UNUSED,
		     symbolS *to_symbol ATTRIBUTE_UNUSED)
{
}


/* Invocation line includes a switch not recognized by the base assembler.
   See if it's a processor-specific option.  */

int
md_parse_option (int c, char *arg)
{
  (void)c;
  (void)arg;
  return 0;
}

void
md_show_usage (FILE *stream)
{
  fprintf (stream, "\
Propeller options\n\
");
}

symbolS *
md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
  return 0;
}

valueT
md_section_align (segT segment ATTRIBUTE_UNUSED,
		  valueT size)
{
  return (size + 1) & ~1;
}

long
md_pcrel_from (fixS *fixP)
{
  return fixP->fx_frag->fr_address + fixP->fx_where + fixP->fx_size;
}

/* Translate internal representation of relocation info to BFD target
   format.  */

arelent *
tc_gen_reloc (asection *section ATTRIBUTE_UNUSED,
	      fixS *fixp)
{
  (void)fixp;
  return 0;
}

