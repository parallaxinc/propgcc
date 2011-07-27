/* Prototypes of target machine functions, Propeller architecture.
   Copyright (C) 2011 Parallax, Inc.
   Contributed by Eric R. Smith <ersmith@totalspectrum.ca>

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 3, or (at your
   option) any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING3.  If not see
   <http://www.gnu.org/licenses/>.  */

#ifndef GCC_PROPELLER_PROTOS_H
#define GCC_PROPELLER_PROTOS_H

extern void propeller_print_operand (FILE *file, rtx op, int letter);
extern void propeller_print_operand_address (FILE *file, rtx addr);
extern void propeller_expand_prologue (void);
extern void propeller_expand_epilogue (void);
extern int propeller_can_use_return (void);

extern bool propeller_legitimate_constant_p (rtx x);
extern bool propeller_legitimate_address_p (enum machine_mode mode, rtx x, bool strict);
extern bool propeller_const_ok_for_letter_p (HOST_WIDE_INT value, int c);

extern HOST_WIDE_INT propeller_initial_elimination_offset (int from, int to);

#if defined TREE_CODE
extern void propeller_init_cumulative_args (CUMULATIVE_ARGS *, tree, rtx, tree);
extern bool propeller_pad_arg_upward (enum machine_mode, const_tree);
extern bool propeller_pad_reg_upward (enum machine_mode, tree, int);
#endif

#endif
