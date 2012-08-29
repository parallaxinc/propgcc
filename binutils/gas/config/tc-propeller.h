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

/* special hack to ignore underscores in constants */
#define IGNORE_UNDERSCORES_IN_INTEGER_CONSTANTS 1

#define md_operand(x)
#define md_number_to_chars number_to_chars_littleendian

long md_chars_to_number (unsigned char *, int);

#define tc_frob_label(s) propeller_frob_label(s)
//#define tc_symbol_new_hook(s) propeller_frob_label(s)

void propeller_frob_label (symbolS * s);

#define TC_S_GET_VALUE propeller_s_get_value
valueT propeller_s_get_value (symbolS *s);

#define tc_fix_adjustable(f) 0

/* end of tc-propeller.h */

/* Stuff for experiments or debugging.  This should all
 * be gone for release */
//#define DEBUG 1
//#define DEBUG2 1
//#define DEBUG3 1
//#define DEBUG4 1
//#define DEBUG5 1
