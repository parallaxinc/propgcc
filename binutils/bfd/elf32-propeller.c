/* propeller-specific support for 32-bit ELF.
   Copyright 2011 Parallax Inc.

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

  /* A 9 bit relocation of the SRC field of an instruction */
  HOWTO (R_PROPELLER_SRC,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 9,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PROPELLER_SRC",	/* name */
	 FALSE,			/* partial_inplace */
	 0x000001FF,		/* src_mask */
	 0x000001FF,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 9 bit relocation of the DST field of an instruction */
  HOWTO (R_PROPELLER_DST,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 9,			/* bitsize */
	 FALSE,			/* pc_relative */
	 9,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PROPELLER_DST",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0003FE00,		/* src_mask */
	 0x0003FE00,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

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
  {BFD_RELOC_PROPELLER_SRC, R_PROPELLER_SRC},
  {BFD_RELOC_PROPELLER_DST, R_PROPELLER_DST},
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
    default:
      r = _bfd_final_link_relocate (howto, input_bfd, input_section,
				    contents, rel->r_offset,
				    relocation, rel->r_addend);
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
	  relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);

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

      if (sec != NULL && elf_discarded_section (sec))
	RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					 rel, relend, howto, contents);

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

#define ELF_ARCH		bfd_arch_propeller
#define ELF_MACHINE_CODE	EM_PROPELLER
#define ELF_MAXPAGESIZE		0x1

#define TARGET_BIG_SYM          bfd_elf32_propeller_vec
#define TARGET_BIG_NAME		"elf32-propeller"

#define elf_info_to_howto_rel			NULL
#define elf_info_to_howto			propeller_info_to_howto_rela
#define elf_backend_relocate_section		propeller_elf_relocate_section
#define elf_backend_gc_mark_hook		propeller_elf_gc_mark_hook
#define elf_backend_check_relocs                propeller_elf_check_relocs

#define elf_backend_can_gc_sections		1
#define elf_backend_rela_normal			1

#define bfd_elf32_bfd_reloc_type_lookup		propeller_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup		propeller_reloc_name_lookup

#include "elf32-target.h"
