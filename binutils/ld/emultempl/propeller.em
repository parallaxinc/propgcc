# This shell script emits a C file. -*- C -*-
#   Copyright 2009  Free Software Foundation, Inc.
#
# This file is part of the GNU Binutils.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
# MA 02110-1301, USA.
#

# This file is sourced from elf32.em, and defines extra propeller-elf
# specific routines.
#
test -z "$TARGET2_TYPE" && TARGET2_TYPE="rel"
fragment <<EOF

static bfd_boolean propeller_build_flag = FALSE;

/* This is a convenient point to tell BFD about target specific flags.
   After the output has been created, but before inputs are read.  */
static void
propeller_elf_create_output_section_statements (void)
{
#if 0
  /* not actually implemented yet */
  extern void bfd_elf32_propeller_set_target_flags (bfd_boolean);

  /* set target specific flags */
  bfd_elf32_propeller_set_target_flags (propeller_build_flag);
#endif
  /* turn off the D_PAGED bit in the output */
  if (link_info.output_bfd)
    link_info.output_bfd->flags &= ~D_PAGED;
}

/*
 * orphan sections whose names start with .cog are actually overlays
 * handle them specially
 */

static lang_output_section_statement_type *
propeller_place_orphan (asection *s, const char *secname, int constraint)
{
  if (!link_info.relocatable && 0 != (s->flags & SEC_ALLOC) )
    {
      /* for now we only put stuff after .text, but we may want to
	 add data overlays some day */
      static struct orphan_save hold[] = 
	{
	  { ".text",
	    SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_CODE,
	    0, 0, 0, 0 },
	  { ".drivers",
	    SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_CODE,
	    0, 0, 0, 0 },
	  { ".rodata",
	    SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_DATA,
	    0, 0, 0, 0 },
	  { ".data",
	    SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_DATA,
	    0, 0, 0, 0 },
	};
      int lastn;
      struct orphan_save *place = NULL;
      lang_output_section_statement_type *after = NULL;

      // if either the first or last 4 characters are .cog, it's a cog
      // overlay
      lastn = strlen (secname) - 4;
      if (!strncmp (secname, ".cog", 4) || (lastn >= 0 && !strcmp (secname + lastn, ".cog")))
	{
	  char *clean, *s2;
	  const char *s1;
	  char *buf;
	  lang_output_section_statement_type *os;
	  lang_memory_region_type *cog_region;

	  //fprintf (stderr, "orphaned section [%s]\n", secname);

	  // hold[0] is for the ".text" section
	  // if there is a .text, put the .cog stuff after it;
	  place = &hold[0];
	  if (place->os == NULL)
	    place->os = lang_output_section_find (place->name);
	  if (place->os == NULL)
	    place->os = lang_output_section_find (place->name);
	  after = place->os;
	  os = lang_insert_orphan (s, secname, constraint, after, place, NULL, NULL);
	  cog_region = lang_memory_region_lookup ("coguser", FALSE);
	  if (cog_region)
	    {
	      os->region = cog_region;
	      os->addr_tree = exp_intop (cog_region->origin);
	      os->lma_region = after->lma_region;
	      os->load_base = NULL;
	    }
	  os->sectype = overlay_section;

	  /* now add the necessary overlay symbols */
	  clean = (char *) xmalloc (strlen (secname) + 1);
	  s2 = clean;
	  for (s1 = secname; *s1 != '\0'; s1++)
	    if (ISALNUM (*s1) || *s1 == '_')
	      *s2++ = *s1;
	  *s2 = '\0';

	  buf = (char *) xmalloc (strlen (clean) + sizeof "__load_start_");
	  sprintf (buf, "__load_start_%s", clean);
	  lang_add_assignment (exp_provide (buf,
					    exp_nameop (LOADADDR, secname),
					    FALSE));

	  buf = (char *) xmalloc (strlen (clean) + sizeof "__load_stop_");
	  sprintf (buf, "__load_stop_%s", clean);
	  lang_add_assignment (exp_provide (buf,
					    exp_binop ('+',
						       exp_nameop (LOADADDR, secname),
						       exp_nameop (SIZEOF, secname)),
					    FALSE));

	  free (clean);
	  return os;
	}
        lastn = strlen (secname) - 6;
	if (!strncmp (secname, ".eecog", 6) || (lastn >= 0 && !strcmp (secname + lastn, ".eecog")))
	{
	  char *clean, *s2;
	  const char *s1;
	  char *buf;
	  lang_output_section_statement_type *os;
	  lang_memory_region_type *cog_region;

	  //fprintf (stderr, "orphaned section [%s]\n", secname);

	  // hold[1] is for the ".drivers" section
	  // if there is a .drivers, put the .cog stuff after it;
	  place = &hold[1];
	  if (place->os == NULL)
	    place->os = lang_output_section_find (place->name);
	  if (place->os == NULL)
	    place->os = lang_output_section_find (place->name);
	  after = place->os;
	  os = lang_insert_orphan (s, secname, constraint, after, place, NULL, NULL);
	  cog_region = lang_memory_region_lookup ("coguser", FALSE);
	  if (cog_region)
	    {
	      os->region = cog_region;
	      os->addr_tree = exp_intop (cog_region->origin);
	      os->lma_region = after->lma_region;
	      os->load_base = NULL;
	    }
	  os->sectype = overlay_section;

	  /* now add the necessary overlay symbols */
	  clean = (char *) xmalloc (strlen (secname) + 1);
	  s2 = clean;
	  for (s1 = secname; *s1 != '\0'; s1++)
	    if (ISALNUM (*s1) || *s1 == '_' )
	      *s2++ = *s1;
	    else if (*s1 == '.')
	      *s2++ = '_';
	  *s2 = '\0';

	  buf = (char *) xmalloc (strlen (clean) + sizeof "__load_start_");
	  sprintf (buf, "__load_start_%s", clean);
	  lang_add_assignment (exp_provide (buf,
					    exp_nameop (LOADADDR, secname),
					    FALSE));

	  buf = (char *) xmalloc (strlen (clean) + sizeof "__load_stop_");
	  sprintf (buf, "__load_stop_%s", clean);
	  lang_add_assignment (exp_provide (buf,
					    exp_binop ('+',
						       exp_nameop (LOADADDR, secname),
						       exp_nameop (SIZEOF, secname)),
					    FALSE));

	  free (clean);
	  return os;
	}
    }
  return gld${EMULATION_NAME}_place_orphan (s, secname, constraint);
}
EOF

# Define some shell vars to insert bits of code into the standard elf
# parse_args and list_options functions.
#
PARSE_AND_LIST_PROLOGUE='
#define OPTION_NO_FLAG_MISMATCH_WARNINGS	301
'

PARSE_AND_LIST_LONGOPTS='
  { "no-flag-mismatch-warnings", no_argument, NULL, OPTION_NO_FLAG_MISMATCH_WARNINGS},
'

PARSE_AND_LIST_OPTIONS='
  fprintf (file, _("  --no-flag-mismatch-warnings Don'\''t warn about objects with incompatible"
		   "                                compiler settings\n"));
'

PARSE_AND_LIST_ARGS_CASES='
    case OPTION_NO_FLAG_MISMATCH_WARNINGS:
      propeller_build_flag = TRUE;
      break;
'

LDEMUL_CREATE_OUTPUT_SECTION_STATEMENTS=propeller_elf_create_output_section_statements
LDEMUL_PLACE_ORPHAN=propeller_place_orphan
