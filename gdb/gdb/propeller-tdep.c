/* Target-dependent code for Parallax Propeller

   Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2007, 2008, 2009,
   2010, 2011 Free Software Foundation, Inc.
   Copyright 2011 Parallax Inc.

   Contributed by Ken Rose, rose@acm.org

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */


#include "defs.h"
#include "frame.h"
#include "frame-unwind.h"
#include "frame-base.h"
#include "dwarf2-frame.h"
#include "trad-frame.h"
#include "symtab.h"
#include "gdbtypes.h"
#include "gdbcmd.h"
#include "gdbcore.h"
#include "gdb_string.h"
#include "value.h"
#include "inferior.h"
#include "dis-asm.h"  
#include "symfile.h"
#include "objfiles.h"
#include "arch-utils.h"
#include "regcache.h"
#include "reggroups.h"

#include "target.h"
#include "opcode/propeller.h"
#include "elf/propeller.h"
#include "elf-bfd.h"

static struct gdbarch *
propeller_gdbarch_init (struct gdbarch_info info,
			struct gdbarch_list *arches)
{
  struct gdbarch *gdbarch;
  struct gdbarch_tdep *tdep;
  int elf_flags;

  //  soft_reg_initialized = 0;

  /* Extract the elf_flags if available.  */
  if (info.abfd != NULL
      && bfd_get_flavour (info.abfd) == bfd_target_elf_flavour)
    elf_flags = elf_elfheader (info.abfd)->e_flags;
  else
    elf_flags = 0;

  /* Try to find a pre-existing architecture.  */
  for (arches = gdbarch_list_lookup_by_info (arches, &info);
       arches != NULL;
       arches = gdbarch_list_lookup_by_info (arches->next, &info))
    {
      //      if (gdbarch_tdep (arches->gdbarch)->elf_flags != elf_flags)
      //	continue;

      return arches->gdbarch;
    }

  /* Need a new architecture.  Fill in a target specific vector.  */
  //  tdep = (struct gdbarch_tdep *) xmalloc (sizeof (struct gdbarch_tdep));
  gdbarch = gdbarch_alloc (&info, tdep);
  //  tdep->elf_flags = elf_flags;
  //  tdep->stack_correction = 1;
  //  tdep->use_page_register = 0;
  //  tdep->prologue = propeller_prologue;
  set_gdbarch_addr_bit (gdbarch, 32);
  //  set_gdbarch_num_pseudo_regs (gdbarch, PROPELLER_NUM_PSEUDO_REGS);
  //  set_gdbarch_pc_regnum (gdbarch, HARD_PC_REGNUM);
  //  set_gdbarch_num_regs (gdbarch, PROPELLER_NUM_REGS);

  /* Initially set everything according to the ABI.
     Use 32-bit integers since it will be the case for most
     programs.  The size of these types should normally be set
     according to the dwarf2 debug information.  */
  set_gdbarch_short_bit (gdbarch, 16);
  set_gdbarch_int_bit (gdbarch, 32);
  set_gdbarch_float_bit (gdbarch, 32);
  set_gdbarch_double_bit (gdbarch, 64);
  set_gdbarch_long_double_bit (gdbarch, 64);
  set_gdbarch_long_bit (gdbarch, 32);
  set_gdbarch_ptr_bit (gdbarch, 32);
  set_gdbarch_long_long_bit (gdbarch, 64);

  /* Characters are signed.  */
  set_gdbarch_char_signed (gdbarch, 1);

  //  set_gdbarch_unwind_pc (gdbarch, propeller_unwind_pc);
  //  set_gdbarch_unwind_sp (gdbarch, propeller_unwind_sp);

  /* Set register info.  */
  set_gdbarch_fp0_regnum (gdbarch, -1);

  //  set_gdbarch_sp_regnum (gdbarch, HARD_SP_REGNUM);
  //  set_gdbarch_register_name (gdbarch, propeller_register_name);
  //  set_gdbarch_register_type (gdbarch, propeller_register_type);
  //  set_gdbarch_pseudo_register_read (gdbarch, propeller_pseudo_register_read);
  //  set_gdbarch_pseudo_register_write (gdbarch, propeller_pseudo_register_write);

  //  set_gdbarch_push_dummy_call (gdbarch, propeller_push_dummy_call);

  //  set_gdbarch_return_value (gdbarch, propeller_return_value);
  //  set_gdbarch_skip_prologue (gdbarch, propeller_skip_prologue);
  set_gdbarch_inner_than (gdbarch, core_addr_lessthan);
  //  set_gdbarch_breakpoint_from_pc (gdbarch, propeller_breakpoint_from_pc);
  //  set_gdbarch_print_insn (gdbarch, gdb_print_insn_propeller);

  //  propeller_add_reggroups (gdbarch);
  //  set_gdbarch_register_reggroup_p (gdbarch, propeller_register_reggroup_p);
  //  set_gdbarch_print_registers_info (gdbarch, propeller_print_registers_info);

  /* Hook in the DWARF CFI frame unwinder.  */
  dwarf2_append_unwinders (gdbarch);

  //  frame_unwind_append_unwinder (gdbarch, &propeller_frame_unwind);
  //  frame_base_set_default (gdbarch, &propeller_frame_base);
  
  /* Methods for saving / extracting a dummy frame's ID.  The ID's
     stack address must match the SP value returned by
     PUSH_DUMMY_CALL, and saved by generic_save_dummy_frame_tos.  */
  //  set_gdbarch_dummy_id (gdbarch, propeller_dummy_id);

  /* Return the unwound PC value.  */
  //  set_gdbarch_unwind_pc (gdbarch, propeller_unwind_pc);

  /* Minsymbol frobbing.  */
  //  set_gdbarch_elf_make_msymbol_special (gdbarch,
  //                                    propeller_elf_make_msymbol_special);

  set_gdbarch_believe_pcc_promotion (gdbarch, 1);

  return gdbarch;
}

/* -Wmissing-prototypes */
extern initialize_file_ftype _initialize_m68hc11_tdep;

void
_initialize_propeller_tdep (void)
{
  register_gdbarch_init (bfd_arch_propeller, propeller_gdbarch_init);
  //  propeller_init_reggroups ();
} 
