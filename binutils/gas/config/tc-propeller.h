/* tc-propeller.h -- Header file for tc-propeller.c.
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
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#define TC_PROPELLER 1

#define TARGET_FORMAT "elf32-propeller"
#define TARGET_ARCH bfd_arch_propeller
#define TARGET_BYTES_BIG_ENDIAN 1

#define TC_KEEP_OPERAND_SPACES 1

#define md_operand(x)

long md_chars_to_number (unsigned char *, int);

/* end of tc-propeller.h */
