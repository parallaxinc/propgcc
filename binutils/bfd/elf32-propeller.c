/* propeller-specific support for 32-bit ELF.
   Copyright 2011-2013 Parallax Inc.

   Copied from elf32-moxie.c which is..
   Copyright 2009, 2010 Free Software Foundation, Inc.

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
#include "elf-bfd.h"
#include "elf/propeller.h"

/* Forward declarations.  */


/* Utilities to actually perform propeller relocs.  */

static bfd_reloc_status_type
propeller_elf_do_pcrel10_reloc (
				reloc_howto_type *howto,
				bfd *abfd,
				asection *input_section,
				bfd_byte *data,
				bfd_vma offset,
				bfd_vma symbol_value,
				bfd_vma addend)
{
  bfd_signed_vma relocation;
  unsigned long x;
  bfd_reloc_status_type status;

  /* Sanity check the address (offset in section).  */
  if (offset > bfd_get_section_limit (abfd, input_section))
    return bfd_reloc_outofrange;

  relocation = symbol_value + addend;
  /* Make it pc relative.  */
  relocation -=	(input_section->output_section->vma
		 + input_section->output_offset);
  if (howto->pcrel_offset)
    relocation -= offset;

  if (relocation < -0x1ff || relocation > 0x1ff)
    status = bfd_reloc_overflow;
  else
    status = bfd_reloc_ok;

  x = bfd_get_32 (abfd, data + offset);
  if (relocation < 0)
    {
      /* toggle add/sub bit */
      x |= 0x04000000;
      relocation = -relocation;
    }
  relocation >>= howto->rightshift;
  relocation <<= howto->bitpos;
  x = (x & ~howto->dst_mask) | (((x & howto->src_mask) + relocation) & howto->dst_mask);
  bfd_put_32 (abfd, (bfd_vma) x, data + offset);

  return status;
}

static bfd_reloc_status_type
propeller_elf_do_inscnt_reloc (
				reloc_howto_type *howto,
				bfd *abfd,
				asection *input_section,
				bfd_byte *data,
				bfd_vma offset,
				bfd_vma symbol_value,
				bfd_vma addend)
{
  bfd_signed_vma relocation;
  unsigned long x;
  bfd_reloc_status_type status;

  /* Sanity check the address (offset in section).  */
  if (offset > bfd_get_section_limit (abfd, input_section))
    return bfd_reloc_outofrange;

  relocation = symbol_value + addend;
  /* Make it pc relative, if necessary */
  if (howto->pc_relative)
      relocation -= (input_section->output_section->vma + input_section->output_offset);
  if (howto->pcrel_offset)
    relocation -= offset;

  /* subtract appropriate offset  */
  switch (howto->type) {
  case R_PROPELLER_REPINSCNT:
      relocation -=	1;
      break;
  case R_PROPELLER_REPSREL:
      relocation -= 4;
      break;
  default:
      break;
  }

  if (relocation < 0) {
      relocation = 0;
      status = bfd_reloc_overflow;
  }

  relocation >>= howto->rightshift;
  /* only 6 bits */
  if (relocation & 0x1f)
    status = bfd_reloc_overflow;
  else
    status = bfd_reloc_ok;

  x = bfd_get_32 (abfd, data + offset);
  relocation <<= howto->bitpos;
  x = (x & ~howto->dst_mask) | (((x & howto->src_mask) + relocation) & howto->dst_mask);
  bfd_put_32 (abfd, (bfd_vma) x, data + offset);

  return status;
}

/* handle propeller specific relocations */
static bfd_reloc_status_type
propeller_specific_reloc (bfd *abfd,
			 arelent *reloc_entry,
			 asymbol * symbol,
			 void *data,
			 asection * input_section,
			 bfd *output_bfd,
			 char **error_message ATTRIBUTE_UNUSED)
{
  /* This part is from bfd_elf_generic_reloc.  */
  if (output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (! reloc_entry->howto->partial_inplace
	  || reloc_entry->addend == 0))
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  if (output_bfd != NULL)
    /* FIXME: See bfd_perform_relocation.  Is this right?  */
    return bfd_reloc_continue;

  switch (reloc_entry->howto->type) {
  case R_PROPELLER_PCREL10:
    return propeller_elf_do_pcrel10_reloc (reloc_entry->howto, abfd,
					   input_section,
					   data, reloc_entry->address,
					   (symbol->value
					    + symbol->section->output_section->vma
					    + symbol->section->output_offset),
					   reloc_entry->addend);
  case R_PROPELLER_REPINSCNT:
  case R_PROPELLER_REPSREL:
    return propeller_elf_do_inscnt_reloc (reloc_entry->howto, abfd,
					   input_section,
					   data, reloc_entry->address,
					   (symbol->value
					    + symbol->section->output_section->vma
					    + symbol->section->output_offset),
					   reloc_entry->addend);
  case R_PROPELLER_23:
  case R_PROPELLER_REPS:
  default:
    return bfd_reloc_notsupported;
  }
}

/* handle the "23" bit relocation (32 bits which may be interpreted
 * as an instruction, and hence must have the conditional execution bits
 * all 0
 */
static bfd_reloc_status_type
propeller_rel23_reloc (bfd *abfd ATTRIBUTE_UNUSED,
			 arelent *reloc_entry,
			 asymbol * symbol,
			 void *data ATTRIBUTE_UNUSED,
			 asection * input_section,
			 bfd *output_bfd,
			 char **error_message ATTRIBUTE_UNUSED)
{
  /* This part is from bfd_elf_generic_reloc; short circuit reloc if
     producing relocatable output and the reloc is against an external
     symbol
   */
  bfd_vma relocation;
  bfd_vma mask;

  if (output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (! reloc_entry->howto->partial_inplace
	  || reloc_entry->addend == 0))
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  if (output_bfd != NULL)
    /* FIXME: See bfd_perform_relocation.  Is this right?  */
    return bfd_reloc_continue;

  /* validate relocation */
  relocation = symbol->value + reloc_entry->addend;

  mask = ~(reloc_entry->howto->dst_mask);
  if (0 != (relocation & mask)) {
    return bfd_reloc_overflow;
  }
  return bfd_reloc_continue;
}

static reloc_howto_type propeller_elf_howto_table[] = {
  /* This reloc does nothing. */
  HOWTO (R_PROPELLER_NONE,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PROPELLER_NONE",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 32 bit absolute relocation. */
  HOWTO (R_PROPELLER_32,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PROPELLER_32",	/* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 32 bit absolute relocation with a "hole" */
  /* This is used for 32 bit data which should be ignored (it's inline
     in code and should be interpreted as a no-op). The conditional
     execution bits must be set such that the data is not executed
  */
  HOWTO (R_PROPELLER_23,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 propeller_rel23_reloc,	/* special_function */
	 "R_PROPELLER_23",	/* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0xffc3ffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 16 bit absolute relocation. */
  HOWTO (R_PROPELLER_16,	/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PROPELLER_16",	/* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* An 8 bit absolute relocation. */
  HOWTO (R_PROPELLER_8,	        /* type */
	 0,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 8,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PROPELLER_8",	/* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x000000ff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 9 bit relocation of the SRC field of an instruction */
  /* this one is an immediate constant rather than an address,
     so do not right shift it by 2
  */
  HOWTO (R_PROPELLER_SRC_IMM,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 9,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PROPELLER_SRC_IMM",	/* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x000001FF,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 9 bit relocation of the SRC field of an instruction */
  HOWTO (R_PROPELLER_SRC,	/* type */
	 2,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 9,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PROPELLER_SRC",	/* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x000001FF,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 9 bit relocation of the DST field of an instruction */
  HOWTO (R_PROPELLER_DST,	/* type */
	 2,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 9,			/* bitsize */
	 FALSE,			/* pc_relative */
	 9,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PROPELLER_DST",	/* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x0003FE00,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* a pc-relative offset between -511 and +511; the sign bit actually
     has to toggle between the "add" and "sub" instructions */
  HOWTO (R_PROPELLER_PCREL10,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 10,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 propeller_specific_reloc,	/* special_function */
	 "R_PROPELLER_PCREL10",	/* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x000001FF,		/* dst_mask */
	 TRUE),		        /* pcrel_offset */

  /* count for REPS instruction
   * REPS uses not only the normal 9 bit dest field for the count, but adds
   * 5 other bits stolen from the condition codes and z bit for a total
   * of 14 bits for the count; also, 1 is subtracted from the count
   * (so a rep count of 0 means to run the loop once)
   */
  HOWTO (R_PROPELLER_REPS,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 14,			/* bitsize */
	 FALSE,			/* pc_relative */
	 9,			/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 propeller_specific_reloc,	/* special_function */
	 "R_PROPELLER_REPS",	/* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x023FFE00,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* a 6 bit absolute instruction repeat count; 1 is subtracted from the
     constant
  */
  HOWTO (R_PROPELLER_REPINSCNT,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 6,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 propeller_specific_reloc,	/* special_function */
	 "R_PROPELLER_REPINSCNT",	/* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x0000003F,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 9 bit relocation of the DST field of an instruction */
  HOWTO (R_PROPELLER_DST_IMM,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 9,			/* bitsize */
	 FALSE,			/* pc_relative */
	 9,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PROPELLER_DST",	/* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x0003FE00,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 32 bit absolute relocation, shifted right by 2 */
  HOWTO (R_PROPELLER_32_DIV4,	/* type */
	 2,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PROPELLER_32_DIV4",	/* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 16 bit absolute relocation, shifted right by 2 */
  HOWTO (R_PROPELLER_16_DIV4,	/* type */
	 2,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PROPELLER_16_DIV4",	/* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* An 8 bit absolute relocation, shifted right by 2 */
  HOWTO (R_PROPELLER_8_DIV4,	        /* type */
	 2,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 8,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PROPELLER_8_DIV4",	/* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x000000ff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 32 bit pc-relative relocation. */
  HOWTO (R_PROPELLER_PCREL32,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PROPELLER_PCREL32",	/* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 TRUE),		        /* pcrel_offset */

  /* A 16 bit pc-relative relocation. */
  HOWTO (R_PROPELLER_PCREL16,	/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PROPELLER_PCREL16",	/* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 TRUE),		        /* pcrel_offset */

  /* An 8 bit pc-relative relocation. */
  HOWTO (R_PROPELLER_PCREL8,	/* type */
	 0,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 8,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PROPELLER_PCREL8",	/* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x000000ff,		/* dst_mask */
	 TRUE), 		/* pcrel_offset */

  /* a 6 bit pcrelative instruction repeat count; 1 is subtracted from the
     constant
  */
  HOWTO (R_PROPELLER_REPSREL,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 7,			/* bitsize */
	 TRUE,	        	/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 propeller_specific_reloc,	/* special_function */
	 "R_PROPELLER_REPSREL",	/* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x0000003F,		/* dst_mask */
	 TRUE),  		/* pcrel_offset */

};

/* Map BFD reloc types to Propeller ELF reloc types. */

struct propeller_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned int propeller_reloc_val;
};

static const struct propeller_reloc_map propeller_reloc_map[] = {
  {BFD_RELOC_NONE, R_PROPELLER_NONE},
  {BFD_RELOC_32, R_PROPELLER_32},
  {BFD_RELOC_16, R_PROPELLER_16},
  {BFD_RELOC_8, R_PROPELLER_8},
  {BFD_RELOC_PROPELLER_SRC_IMM, R_PROPELLER_SRC_IMM},
  {BFD_RELOC_PROPELLER_SRC, R_PROPELLER_SRC},
  {BFD_RELOC_PROPELLER_DST, R_PROPELLER_DST},
  {BFD_RELOC_PROPELLER_DST_IMM, R_PROPELLER_DST_IMM},
  {BFD_RELOC_PROPELLER_23, R_PROPELLER_23},
  {BFD_RELOC_PROPELLER_PCREL10, R_PROPELLER_PCREL10},
  {BFD_RELOC_PROPELLER_REPS, R_PROPELLER_REPS},
  {BFD_RELOC_PROPELLER_REPINSCNT, R_PROPELLER_REPINSCNT},
  {BFD_RELOC_PROPELLER_32_DIV4, R_PROPELLER_32_DIV4},
  {BFD_RELOC_PROPELLER_16_DIV4, R_PROPELLER_16_DIV4},
  {BFD_RELOC_PROPELLER_8_DIV4, R_PROPELLER_8_DIV4},
  {BFD_RELOC_32_PCREL, R_PROPELLER_PCREL32},
  {BFD_RELOC_16_PCREL, R_PROPELLER_PCREL16},
  {BFD_RELOC_8_PCREL, R_PROPELLER_PCREL8},
  {BFD_RELOC_PROPELLER_REPSREL, R_PROPELLER_REPSREL},
};

static reloc_howto_type *
propeller_reloc_type_lookup (bfd * abfd ATTRIBUTE_UNUSED,
			     bfd_reloc_code_real_type code)
{
  unsigned int i;

  for (i = sizeof (propeller_reloc_map) / sizeof (propeller_reloc_map[0]);
       --i;)
    if (propeller_reloc_map[i].bfd_reloc_val == code)
      return &propeller_elf_howto_table[propeller_reloc_map[i].
					propeller_reloc_val];

  return NULL;
}

static reloc_howto_type *
propeller_reloc_name_lookup (bfd * abfd ATTRIBUTE_UNUSED, const char *r_name)
{
  unsigned int i;

  for (i = 0;
       i <
       sizeof (propeller_elf_howto_table) /
       sizeof (propeller_elf_howto_table[0]); i++)
    if (propeller_elf_howto_table[i].name != NULL
	&& strcasecmp (propeller_elf_howto_table[i].name, r_name) == 0)
      return &propeller_elf_howto_table[i];

  return NULL;
}

/* Set the howto pointer for an PROPELLER ELF reloc.  */

static void
propeller_info_to_howto_rela (bfd * abfd ATTRIBUTE_UNUSED,
			      arelent * cache_ptr, Elf_Internal_Rela * dst)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  BFD_ASSERT (r_type < (unsigned int) R_PROPELLER_max);
  cache_ptr->howto = &propeller_elf_howto_table[r_type];
}

/* Perform a single relocation.  By default we use the standard BFD
   routines, but a few relocs, we have to do them ourselves.  */

static bfd_reloc_status_type
propeller_final_link_relocate (reloc_howto_type * howto,
			       bfd * input_bfd,
			       asection * input_section,
			       bfd_byte * contents,
			       Elf_Internal_Rela * rel, bfd_vma relocation)
{
  bfd_reloc_status_type r = bfd_reloc_ok;

  switch (howto->type)
    {
    case R_PROPELLER_23:
      /* do a sanity check */
      if ((relocation & ~howto->dst_mask) != 0) {
	return bfd_reloc_overflow;
      }
      /* fall through */
    case R_PROPELLER_SRC_IMM:
    case R_PROPELLER_SRC:
    case R_PROPELLER_DST:
    case R_PROPELLER_32:
    case R_PROPELLER_16:
    case R_PROPELLER_8:
    case R_PROPELLER_32_DIV4:
    case R_PROPELLER_16_DIV4:
    case R_PROPELLER_8_DIV4:
    case R_PROPELLER_PCREL32:
    case R_PROPELLER_PCREL16:
    case R_PROPELLER_PCREL8:
      r = _bfd_final_link_relocate (howto, input_bfd, input_section,
				    contents, rel->r_offset,
				    relocation, rel->r_addend);
      break;
    case R_PROPELLER_PCREL10:
      r = propeller_elf_do_pcrel10_reloc(howto, input_bfd, input_section,
					 contents, rel->r_offset,
					 relocation, rel->r_addend);
      break;
    case R_PROPELLER_REPINSCNT:
      r = propeller_elf_do_inscnt_reloc(howto, input_bfd, input_section,
					 contents, rel->r_offset,
					 relocation, rel->r_addend);
      break;
    default:
      r = bfd_reloc_notsupported;
      break;
    }

  return r;
}

/* Relocate an PROPELLER ELF section.

   The RELOCATE_SECTION function is called by the new ELF backend linker
   to handle the relocations for a section.

   The relocs are always passed as Rela structures; if the section
   actually uses Rel structures, the r_addend field will always be
   zero.

   This function is responsible for adjusting the section contents as
   necessary, and (if using Rela relocs and generating a relocatable
   output file) adjusting the reloc addend as necessary.

   This function does not have to worry about setting the reloc
   address or the reloc symbol index.

   LOCAL_SYMS is a pointer to the swapped in local symbols.

   LOCAL_SECTIONS is an array giving the section in the input file
   corresponding to the st_shndx field of each local symbol.

   The global hash table entry for the global symbols can be found
   via elf_sym_hashes (input_bfd).

   When generating relocatable output, this function must handle
   STB_LOCAL/STT_SECTION symbols specially.  The output symbol is
   going to be the section symbol corresponding to the output
   section, which means that the addend must be adjusted
   accordingly.  */

static bfd_boolean
propeller_elf_relocate_section (bfd * output_bfd,
				struct bfd_link_info *info,
				bfd * input_bfd,
				asection * input_section,
				bfd_byte * contents,
				Elf_Internal_Rela * relocs,
				Elf_Internal_Sym * local_syms,
				asection ** local_sections)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;

  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);
  relend = relocs + input_section->reloc_count;

  for (rel = relocs; rel < relend; rel++)
    {
      reloc_howto_type *howto;
      unsigned long r_symndx;
      Elf_Internal_Sym *sym;
      asection *sec;
      struct elf_link_hash_entry *h;
      bfd_vma relocation;
      bfd_reloc_status_type r;
      const char *name;
      int r_type;

      r_type = ELF32_R_TYPE (rel->r_info);
      r_symndx = ELF32_R_SYM (rel->r_info);
      howto = propeller_elf_howto_table + r_type;
      h = NULL;
      sym = NULL;
      sec = NULL;

      if (r_symndx < symtab_hdr->sh_info)
	{
	  sym = local_syms + r_symndx;
	  sec = local_sections[r_symndx];
	  if( (r_type == R_PROPELLER_SRC_IMM) && 
	      0 != (sym->st_other & PROPELLER_OTHER_COG_RAM) )
	    {
	      Elf_Internal_Sym s = *sym;
	      s.st_value /= 4;
	      relocation = _bfd_elf_rela_local_sym (output_bfd, &s, &sec, rel);
	    }
	  else
	    {
	      relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);
	    }

	  name = bfd_elf_string_from_elf_section
	    (input_bfd, symtab_hdr->sh_link, sym->st_name);
	  name = (name == NULL) ? bfd_section_name (input_bfd, sec) : name;
	}
      else
	{
	  bfd_boolean unresolved_reloc, warned;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes,
				   h, sec, relocation,
				   unresolved_reloc, warned);

	  name = h->root.root.string;
	}

      if (sec != NULL && discarded_section (sec))
	RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					 rel, 1, relend, howto, 0, contents);

      if (info->relocatable)
	continue;

      r = propeller_final_link_relocate (howto, input_bfd, input_section,
					 contents, rel, relocation);

      if (r != bfd_reloc_ok)
	{
	  const char *msg = NULL;

	  switch (r)
	    {
	    case bfd_reloc_overflow:
	      r = info->callbacks->reloc_overflow
		(info, (h ? &h->root : NULL), name, howto->name,
		 (bfd_vma) 0, input_bfd, input_section, rel->r_offset);
	      break;

	    case bfd_reloc_undefined:
	      r = info->callbacks->undefined_symbol
		(info, name, input_bfd, input_section, rel->r_offset, TRUE);
	      break;

	    case bfd_reloc_outofrange:
	      msg = _("internal error: out of range error");
	      break;

	    case bfd_reloc_notsupported:
	      msg = _("internal error: unsupported relocation error");
	      break;

	    case bfd_reloc_dangerous:
	      msg = _("internal error: dangerous relocation");
	      break;

	    default:
	      msg = _("internal error: unknown error");
	      break;
	    }

	  if (msg)
	    r = info->callbacks->warning
	      (info, msg, name, input_bfd, input_section, rel->r_offset);

	  if (!r)
	    return FALSE;
	}
    }

  return TRUE;
}

/* Merge non-visibility st_other attributes. Ensure that our
   architecture specific flags are copied into h->other even if this
   is not a definition of the symbol
*/
static void
propeller_elf_merge_symbol_attribute (struct elf_link_hash_entry *h,
				      const Elf_Internal_Sym *isym,
				      bfd_boolean definition,
				      bfd_boolean dynamic ATTRIBUTE_UNUSED)
{
  if ((isym->st_other & ~ELF_ST_VISIBILITY (-1)) != 0)
    {
      unsigned char other;

      other = (definition ? isym->st_other : h->other);
      other &= ~ELF_ST_VISIBILITY (-1);
      h->other = other | ELF_ST_VISIBILITY (h->other);
    }
  if (!definition && (isym->st_other & PROPELLER_OTHER_FLAGS))
    h->other |= isym->st_other & PROPELLER_OTHER_FLAGS;
}


/* Return the section that should be marked against GC for a given
   relocation.  */

static asection *
propeller_elf_gc_mark_hook (asection * sec,
			    struct bfd_link_info *info,
			    Elf_Internal_Rela * rel,
			    struct elf_link_hash_entry *h,
			    Elf_Internal_Sym * sym)
{
  return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);
}

/* Look through the relocs for a section during the first phase.
   Since we don't do .gots or .plts, we just need to consider the
   virtual table relocs for gc.  */

static bfd_boolean
propeller_elf_check_relocs (bfd * abfd,
			    struct bfd_link_info *info,
			    asection * sec, const Elf_Internal_Rela * relocs)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;

  if (info->relocatable)
    return TRUE;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      struct elf_link_hash_entry *h;
      unsigned long r_symndx;

      r_symndx = ELF32_R_SYM (rel->r_info);
      if (r_symndx < symtab_hdr->sh_info)
	h = NULL;
      else
	{
	  h = sym_hashes[r_symndx - symtab_hdr->sh_info];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;
	}
    }

  return TRUE;
}

/* Propeller ELF local labels start with either '.' or 'L_'.  */

static bfd_boolean
propeller_elf_is_local_label_name (bfd *abfd, const char *name)
{
#if 0
  if (name[0] == 'L' && name[1] == '_')
    return TRUE;
#endif
  /* accept the generic ELF local label syntax as well.  */
  return _bfd_elf_is_local_label_name (abfd, name);
}

/* Set the right machine number for an ELF file */
static bfd_boolean
propeller_elf_object_p (bfd *abfd)
{
  flagword flags;
  unsigned long mach;

  flags = elf_elfheader (abfd)->e_flags;
  if (flags & EF_PROPELLER_PROP2)
    {
      mach = bfd_mach_prop2;
    }
  else
    {
      /* default to propeller 1 for compatibility with old object files */
      mach = bfd_mach_prop1;
    }
  bfd_default_set_arch_mach (abfd, bfd_arch_propeller, mach);
  return TRUE;
}

/* final write processing; set flags in the object file, etc */
static void
propeller_elf_final_write_processing (bfd *abfd,
				      bfd_boolean linker ATTRIBUTE_UNUSED)
{
  flagword val;
  /* set the flags */
  switch (bfd_get_mach (abfd))
    {
    case bfd_mach_prop1:
      val = EF_PROPELLER_PROP1;
      break;
    case bfd_mach_prop2:
      val = EF_PROPELLER_PROP2;
      break;
    default:
      /* leave flags unchanged */
      val = 0;
      break;
    }
  if (val != 0) {
    elf_elfheader (abfd)->e_flags &= ~(EF_PROPELLER_MACH);
    elf_elfheader (abfd)->e_flags |= val;
  }
}

/* Function to set the ELF flag bits.  */

static bfd_boolean
propeller_elf_set_private_flags (bfd * abfd, flagword flags)
{
  elf_elfheader (abfd)->e_flags = flags;
  elf_flags_init (abfd) = TRUE;
  return TRUE;
}

static bfd_boolean no_warn_mismatch = FALSE;

/* provide a prototype to make the compiler happy */
void bfd_elf32_propeller_set_target_flags (bfd_boolean);

void
bfd_elf32_propeller_set_target_flags (bfd_boolean user_no_warn_mismatch)
{
  no_warn_mismatch = user_no_warn_mismatch;
}
/* Merge backend specific data from an object file to the output
   object file when linking.  */

static bfd_boolean
propeller_elf_merge_private_bfd_data (bfd * ibfd, bfd * obfd)
{
  flagword old_flags;
  flagword new_flags;
  bfd_boolean error = FALSE;

  new_flags = elf_elfheader (ibfd)->e_flags;
  old_flags = elf_elfheader (obfd)->e_flags;

  if (!elf_flags_init (obfd))
    {
      /* First call, no flags set.  */
      elf_flags_init (obfd) = TRUE;
      elf_elfheader (obfd)->e_flags = new_flags;
    }
  else if (old_flags != new_flags)
    {
      flagword old_mach, new_mach;
      flagword old_ef, new_ef;

      /* we can check here for mismatches in bits */
      old_mach = old_flags & EF_PROPELLER_MACH;
      new_mach = new_flags & EF_PROPELLER_MACH;
      old_ef = old_flags & (~EF_PROPELLER_MACH);
      new_ef = new_flags & (~EF_PROPELLER_MACH);

      if (old_mach != 0 && new_mach != 0 && old_mach != new_mach)
	{
	      (*_bfd_error_handler)
		("propeller architecture mismatch: old = 0x%.8lx, new = 0x%.8lx, filename = %s",
		 old_mach, new_mach, bfd_get_filename (ibfd));
	}
      if (old_ef != 0 && new_ef != 0)
	{
	  /* Only complain if inconsistent bits are being set */
	  if (no_warn_mismatch)
	    {
	      elf_elfheader (obfd)->e_flags = (new_flags | old_flags);
	    }
	  else
	    {
	      (*_bfd_error_handler)
		("ELF header flags mismatch: old_flags = 0x%.8lx, new_flags = 0x%.8lx, filename = %s",
		 old_flags, new_flags, bfd_get_filename (ibfd));
	      error = TRUE;
	    }
	}
      else
	elf_elfheader (obfd)->e_flags |= new_flags;
    }

  if (error)
    bfd_set_error (bfd_error_bad_value);

  return !error;
}

static bfd_boolean
propeller_elf_print_private_bfd_data (bfd * abfd, void * ptr)
{
  FILE * file = (FILE *) ptr;
  flagword flags;

  BFD_ASSERT (abfd != NULL && ptr != NULL);

  /* Print normal ELF private data.  */
  _bfd_elf_print_private_bfd_data (abfd, ptr);

  flags = elf_elfheader (abfd)->e_flags;
  fprintf (file, _("private flags = 0x%lx:"), (long) flags);

  if (flags & EF_PROPELLER_PROP1)
    fprintf (file, _(" [prop1]"));
  if (flags & EF_PROPELLER_PROP2)
    fprintf (file, _(" [prop2]"));
  if (flags & EF_PROPELLER_COMPRESS)
    fprintf (file, _(" [cmm]"));
  if (flags & EF_PROPELLER_XMM)
    fprintf (file, _(" [xmm]"));
  if (flags & EF_PROPELLER_ABI_VERS)
    fprintf (file, _("[abi version %d]"), EF_PROPELLER_GET_ABI(flags));
  fputc ('\n', file);
  return TRUE;
}

/* Tweak phdrs before writing them out.  */
/* for the propeller, this just involves copying various flags from the
   section headers
*/

#define PROP_FLAGS (0xf0000000)

static int
propeller_elf_modify_program_headers (bfd *abfd, struct bfd_link_info *info)
{
  struct elf_obj_tdata *tdata;
  Elf_Internal_Phdr *phdr;
  unsigned int i;
  unsigned int flags;
  struct elf_segment_map *m;

  if (info == NULL)
    return TRUE;

  tdata = elf_tdata (abfd);
  phdr = tdata->phdr;
  for (i = 0, m = elf_tdata (abfd)->segment_map; m; ++i, m = m->next)
    {
      if (m->count != 0
	  && (flags = elf_section_flags (m->sections[0])) != 0)
	{
	  /* Copy processor specific flags  */
	  phdr[i].p_flags |= flags & PROP_FLAGS;
	}
    }

  return TRUE;
}

static void
propeller_elf_gc_keep (struct bfd_link_info *info)
{
  bfd *in;
  struct bfd_section *sec;
  /*
    look for sections starting or ending with ".cog";
    these should never be garbage collected
    (actually we could do even better and KEEP them only
    if the corresponding _load_start_xxx symbol is referenced,
    but for now just keep them all)
  */
  for (in = info->input_bfds; in; in = in->link_next)
    {
      for (sec = in->sections; sec; sec = sec->next)
	{
	  const char *name = sec->name;
	  size_t namelen = strlen(name);
	  int keep = 0;

	  if (namelen > 4 && !strcmp(name+namelen-4, ".cog"))
	    keep = 1;
	  else if (!strncmp(name, ".cog", 4))
	    keep = 1;
	  if (keep)
	    sec->flags |= SEC_KEEP;
	}
    }
  /* do the usual marking of sections containing entry symbol and
     symbols undefined on the command-line
  */
  _bfd_elf_gc_keep (info);
}

#define ELF_ARCH		bfd_arch_propeller
#define ELF_MACHINE_CODE	EM_PROPELLER
#define ELF_MAXPAGESIZE		0x1

#define TARGET_LITTLE_SYM          bfd_elf32_propeller_vec
#define TARGET_LITTLE_NAME		"elf32-propeller"

#define elf_info_to_howto_rel			NULL
#define elf_info_to_howto			propeller_info_to_howto_rela
#define elf_backend_relocate_section		propeller_elf_relocate_section
#define elf_backend_gc_mark_hook		propeller_elf_gc_mark_hook
#define elf_backend_check_relocs                propeller_elf_check_relocs
#define elf_backend_merge_symbol_attribute      propeller_elf_merge_symbol_attribute
#define elf_backend_modify_program_headers      propeller_elf_modify_program_headers
#define elf_backend_object_p                    propeller_elf_object_p
#define elf_backend_final_write_processing      propeller_elf_final_write_processing
#define elf_backend_gc_keep                     propeller_elf_gc_keep

#define elf_backend_can_gc_sections		1
#define elf_backend_rela_normal			1

#define bfd_elf32_bfd_reloc_type_lookup		propeller_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup		propeller_reloc_name_lookup
#define bfd_elf32_bfd_set_private_flags         propeller_elf_set_private_flags
#define bfd_elf32_bfd_merge_private_bfd_data    propeller_elf_merge_private_bfd_data
#define bfd_elf32_bfd_print_private_bfd_data    propeller_elf_print_private_bfd_data

#define bfd_elf32_bfd_is_local_label_name \
					propeller_elf_is_local_label_name

#include "elf32-target.h"
