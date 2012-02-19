/* Propeller ELF support for BFD.
   Copyright 2011 Parallax Inc.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

#ifndef _ELF_PROPELLER_H
#define _ELF_PROPELLER_H

/* Flags for the st_other field. */
#define PROPELLER_OTHER_COG_RAM 0x80
#define PROPELLER_OTHER_FLAGS 0x80

#include "elf/reloc-macros.h"

/* Relocation types.  */
START_RELOC_NUMBERS (elf_propeller_reloc_type)
  RELOC_NUMBER (R_PROPELLER_NONE, 0)
  RELOC_NUMBER (R_PROPELLER_32, 1)
  RELOC_NUMBER (R_PROPELLER_23, 2)
  RELOC_NUMBER (R_PROPELLER_16, 3)
  RELOC_NUMBER (R_PROPELLER_8, 4)
  RELOC_NUMBER (R_PROPELLER_SRC_IMM, 5)
  RELOC_NUMBER (R_PROPELLER_SRC, 6)
  RELOC_NUMBER (R_PROPELLER_DST, 7)
  RELOC_NUMBER (R_PROPELLER_PCREL10, 8)
END_RELOC_NUMBERS (R_PROPELLER_max)

#endif /* _ELF_PROPELLER_H */
