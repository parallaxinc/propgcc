/* tc-propeller.h -- Header file for tc-propeller.c.
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
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#define TC_PROPELLER 1

#define TARGET_FORMAT "elf32-propeller"
#define TARGET_ARCH bfd_arch_propeller
#define TARGET_BYTES_BIG_ENDIAN 0
#define TC_SYMFIELD_TYPE int
#define LABELS_WITHOUT_COLONS 1

#define TC_KEEP_OPERAND_SPACES 1
#define NO_PSEUDO_DOT 1
/* allow $ for hex and % for bin */
#define LITERAL_PREFIXDOLLAR_HEX
#define LITERAL_PREFIXPERCENT_BIN

/* allow Sun style dollar labels */
#define LOCAL_LABELS_DOLLAR 1

/* special hack to ignore underscores in constants */
#define IGNORE_UNDERSCORES_IN_INTEGER_CONSTANTS 1

#define md_operand(x)
#define md_number_to_chars number_to_chars_littleendian

long md_chars_to_number (unsigned char *, int);

#define tc_frob_label(s) propeller_frob_label(s)
//#define tc_symbol_new_hook(s) propeller_frob_label(s)

void propeller_frob_label (symbolS * s);

#define tc_fix_adjustable(f) 0

#define elf_tc_final_processing propeller_elf_final_processing
void propeller_elf_final_processing (void);

#define TC_PARSE_CONS_EXPRESSION(EXP, NBYTES) propeller_cons (EXP, NBYTES)
#define TC_CONS_FIX_NEW propeller_cons_fix_new

void propeller_cons (expressionS *, int);
void propeller_cons_fix_new (struct frag *, int, unsigned int, struct expressionS *);

#define md_start_line_hook propeller_start_line_hook
void propeller_start_line_hook (void);

extern const char propeller_symbol_chars[];
#define tc_symbol_chars propeller_symbol_chars

/* end of tc-propeller.h */

