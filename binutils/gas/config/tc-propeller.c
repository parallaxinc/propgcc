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


/* These chars start a comment anywhere in a source file (except inside
   another comment.  */
const char comment_chars[] = "#/";

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


const char *md_shortopts = "m:";

struct option md_longopts[] =
{
#define OPTION_CPU 257
  { "cpu", required_argument, NULL, OPTION_CPU },
#define OPTION_MACHINE 258
  { "machine", required_argument, NULL, OPTION_MACHINE },
#define OPTION_PIC 259
  { "pic", no_argument, NULL, OPTION_PIC },
  { NULL, no_argument, NULL, 0 }
};

size_t md_longopts_size = sizeof (md_longopts);


void
md_begin (void)
{
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

void
md_assemble (char *instruction_string)
{
  (void)instruction_string;
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

int md_short_jump_size = 2;
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

