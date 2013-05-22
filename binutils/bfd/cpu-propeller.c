/* BFD support for the Parallax Propeller architecture.
   Copyright 2011 Parallax Inc.
   Contributed by Eric R. Smith  <ersmith@totalspectrum.ca>

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
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"

#define PROP(mach, print_name, default_p, next) \
 {					\
    32,	/* 32 bits in a word  */	\
    32,	/* 32 bits in an address  */	\
    8,	/* 8 bits in a byte  */		\
    bfd_arch_propeller,			\
    mach, /* machine number */				\
    "propeller",				\
    print_name,				\
    4, /* section alignment power  */	\
    default_p,				\
    bfd_default_compatible,		\
    bfd_default_scan,			\
    bfd_arch_default_fill,		\
    next,				\
  }

static const bfd_arch_info_type arch_info_struct[] =
{
    PROP( bfd_mach_prop1, "prop1", FALSE, &arch_info_struct[1] ),
    PROP( bfd_mach_prop2, "prop2", FALSE, NULL )
};

/* the default architecture is prop1 but with a machine number of 0,
   to allow the linker to distinguish between a default setting of
   "propeller" and an explicit "prop1"
*/
const bfd_arch_info_type bfd_propeller_arch =
  PROP( 0, "prop", TRUE, &arch_info_struct[0] );
