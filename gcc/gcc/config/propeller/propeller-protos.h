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

extern void propeller_output_label (FILE *file, const char * name);
extern void propeller_print_operand (FILE *file, rtx op, int letter);
extern void propeller_print_operand_address (FILE *file, rtx addr);
extern bool propeller_print_operand_punct_valid_p (unsigned char code);
extern void propeller_output_seqend (FILE *);
extern void propeller_weaken_label (FILE *file, const char *name);
extern void propeller_expand_prologue (void);
extern void propeller_expand_epilogue (bool is_sibcall);
extern int propeller_can_use_return (void);

extern bool propeller_cogaddr_p (rtx x);
extern bool propeller_cogmem_p (rtx op);
extern bool propeller_stack_operand_p (rtx op);
extern bool propeller_legitimate_constant_p (rtx x);
extern bool propeller_legitimate_address_p (enum machine_mode mode, rtx x, bool strict);
extern bool propeller_const_ok_for_letter_p (HOST_WIDE_INT value, int c);

extern HOST_WIDE_INT propeller_initial_elimination_offset (int from, int to);

#if defined(TREE_CODE)
extern void propeller_init_cumulative_args (CUMULATIVE_ARGS *, tree, rtx, tree);
extern bool propeller_pad_arg_upward (enum machine_mode, const_tree);
extern bool propeller_pad_reg_upward (enum machine_mode, tree, int);

extern void propeller_asm_output_aligned_common (FILE *, tree, const char *,
						 int, int, int);

extern void propeller_declare_function_name (FILE *, const char *, tree);
#endif

extern bool propeller_modes_tieable_p (enum machine_mode, enum machine_mode);
extern bool propeller_hard_regno_mode_ok (unsigned int, enum machine_mode);
extern bool propeller_epilogue_uses(int);

extern bool propeller_match_ccmode (rtx, enum machine_mode);

#if defined(RTX_CODE)
extern enum machine_mode propeller_select_cc_mode (enum rtx_code, rtx, rtx);
extern rtx propeller_gen_compare_reg (enum rtx_code, rtx, rtx);
extern RTX_CODE propeller_canonicalize_comparison (RTX_CODE, rtx *, rtx *);
extern rtx propeller_return_addr (int, rtx);
#endif

extern bool propeller_expand_call (rtx, rtx, rtx, bool);
extern bool propeller_forward_branch_p (rtx);

extern void propeller_emit_stack_pushm (rtx *operands);
extern void propeller_emit_stack_popm (rtx *operands, int doretaswell);

extern int propeller_reg_dead_peep (rtx first, rtx reg);

/* some variable declarations; we put them here rather than in propeller.h
 * so that the libgcc build doesn't see them
 */
/* some variables controlling output of constants and functions */
extern bool propeller_need_mulsi;
extern bool propeller_need_udivsi;
extern bool propeller_need_divsi;
extern bool propeller_need_clzsi;
extern bool propeller_need_cmpswapsi;
extern bool propeller_need_mask0000ffff;
extern bool propeller_need_maskffffffff;


#endif
