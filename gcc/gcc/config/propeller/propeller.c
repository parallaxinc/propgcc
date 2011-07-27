/* Target Code for Parallax Propeller
   Copyright (C) 2011 Parallax, Inc.
   Copyright (C) 2008, 2009, 2010  Free Software Foundation
   Contributed by Eric R. Smith.
   Initial code based on moxie.c, contributed by Anthony Green.

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

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"
#include "regs.h"
#include "hard-reg-set.h"
#include "insn-config.h"
#include "conditions.h"
#include "insn-flags.h"
#include "output.h"
#include "insn-attr.h"
#include "flags.h"
#include "recog.h"
#include "reload.h"
#include "diagnostic-core.h"
#include "obstack.h"
#include "tree.h"
#include "expr.h"
#include "optabs.h"
#include "except.h"
#include "function.h"
#include "ggc.h"
#include "target.h"
#include "target-def.h"
#include "tm_p.h"
#include "langhooks.h"
#include "df.h"

struct propeller_frame_info
{
  HOST_WIDE_INT total_size;	/* number of bytes of entire frame.  */
  HOST_WIDE_INT callee_size;	/* number of bytes to save callee saves.  */
  HOST_WIDE_INT pretend_size;	/* number of bytes we pretend caller did.  */
  HOST_WIDE_INT args_size;	/* number of bytes for outgoing arguments.  */
  HOST_WIDE_INT locals_size;	/* number of bytes for local variables.  */
  unsigned int reg_save_mask;	/* mask of saved registers.  */
};

/* Current frame information calculated by compute_frame_size.  */
static struct propeller_frame_info current_frame_info;
static HOST_WIDE_INT propeller_compute_frame_size (int size);

#undef TARGET_LEGITIMATE_ADDRESS_P
#define TARGET_LEGITIMATE_ADDRESS_P propeller_legitimate_address_p

struct gcc_target targetm = TARGET_INITIALIZER;



/* The PRINT_OPERAND worker.  */

void
propeller_print_operand (FILE * file, rtx op, int letter)
{
  enum rtx_code code;

  code = GET_CODE (op);

  if (code == SIGN_EXTEND)
    op = XEXP (op, 0), code = GET_CODE (op);
  else if (code == REG || code == SUBREG)
    {
      int regnum;

      if (code == REG)
	regnum = REGNO (op);
      else
	regnum = true_regnum (op);

      fprintf (file, "%s", reg_names[regnum]);
    }
  else if (code == HIGH)
    output_addr_const (file, XEXP (op, 0));  
  else if (code == MEM)
    output_address (XEXP (op, 0));
  else if (GET_CODE (op) == CONST_DOUBLE)
    {
      if ((CONST_DOUBLE_LOW (op) != 0) || (CONST_DOUBLE_HIGH (op) != 0))
	output_operand_lossage ("only 0.0 can be loaded as an immediate");
      else
	fprintf (file, "0");
    }
  else if (code == EQ)
    fprintf (file, "e  ");
  else if (code == NE)
    fprintf (file, "ne ");
  else if (code == GT)
    fprintf (file, "g  ");
  else if (code == GTU)
    fprintf (file, "gu ");
  else if (code == LT)
    fprintf (file, "l  ");
  else if (code == LTU)
    fprintf (file, "lu ");
  else if (code == GE)
    fprintf (file, "ge ");
  else if (code == GEU)
    fprintf (file, "geu");
  else if (code == LE)
    fprintf (file, "le ");
  else if (code == LEU)
    fprintf (file, "leu");
  else
    output_addr_const (file, op);
}

/* A C compound statement to output to stdio stream STREAM the
   assembler syntax for an instruction operand that is a memory
   reference whose address is ADDR.  ADDR is an RTL expression.

   On some machines, the syntax for a symbolic address depends on
   the section that the address refers to.  On these machines,
   define the macro `ENCODE_SECTION_INFO' to store the information
   into the `symbol_ref', and then check for it here.  */

void
propeller_print_operand_address (FILE * file, rtx addr)
{
  switch (GET_CODE (addr))
    {
    case REG:
      fprintf (file, "%s", reg_names[REGNO (addr)]);
      break;

    case MEM:
      output_address (XEXP (addr, 0));
      break;

    case SYMBOL_REF:
	  output_addr_const (file, addr);
      break;

    default:
      fatal_insn ("invalid addressing mode", addr);
      break;
    }
}

/*
 * check for legitimate addresses
 */
bool
propeller_legitimate_address_p (enum machine_mode mode, rtx addr, bool strict)
{
    /* the only kind of address the propeller 1 supports is register indirect */
    while (GET_CODE (addr) == SUBREG)
        addr = SUBREG_REG (addr);
    if (GET_CODE (addr) == REG) {
        int regno = REGNO(addr);
        if (HARD_REGNO_OK_FOR_BASE_P(regno))
            return true;
        if (strict && HARD_REGNO_OK_FOR_BASE_P(reg_renumber[regno]))
            return true;
        return regno >= FIRST_PSEUDO_REGISTER;
    }
    /* allow symbol references for calls */
    if (mode == VOIDmode && GET_CODE (addr) == SYMBOL_REF)
        return true;

    return false;
}


/* utility functions */
/* Generate an emit a word sized add instruction.  */

static rtx
emit_add (rtx dest, rtx src0, rtx src1)
{
  rtx insn;
  insn = emit_insn (gen_addsi3 (dest, src0, src1));
  return insn;
}


/*
 * stack related functions
 */

HOST_WIDE_INT
propeller_initial_elimination_offset (int from, int to)
{
  HOST_WIDE_INT offset = 0;

  switch (from)
  {
  case ARG_POINTER_REGNUM:
      switch (to) {
      case FRAME_POINTER_REGNUM:
          offset = 0;
          break;
      case STACK_POINTER_REGNUM:
          offset = propeller_compute_frame_size (get_frame_size ()) - current_frame_info.pretend_size;
          break;
      default:
          gcc_unreachable ();
          break;
      }
  default:
      gcc_unreachable ();
      break;
  }
}
/* Generate and emit RTL to save or restore callee save registers.  */
static void
expand_save_restore (struct propeller_frame_info *info, int op)
{
  unsigned int reg_save_mask = info->reg_save_mask;
  int regno;
  HOST_WIDE_INT offset;
  rtx insn;

  /* Callee saves are below locals and above outgoing arguments.  */
  offset = info->args_size + info->callee_size;
  for (regno = 0; regno <= 15; regno++)
    {
      if ((reg_save_mask & (1 << regno)) != 0)
	{
	  rtx offset_rtx;
	  rtx mem;
	  
	  offset_rtx = GEN_INT (offset);
            {
              /* r7 is caller saved so it can be used as a temp reg.  */
              rtx r7;
               
              r7 = gen_rtx_REG (word_mode, 7);
              insn = emit_move_insn (r7, offset_rtx);
              if (op == 0)
                RTX_FRAME_RELATED_P (insn) = 1;
              insn = emit_add (r7, r7, stack_pointer_rtx);
              if (op == 0)
                RTX_FRAME_RELATED_P (insn) = 1;                
              mem = gen_rtx_MEM (word_mode, r7);
            }                                                 	    
	    	    
	  if (op == 0)
	    insn = emit_move_insn (mem, gen_rtx_REG (word_mode, regno));
	  else
	    insn = emit_move_insn (gen_rtx_REG (word_mode, regno), mem);
        
	  /* only prologue instructions which set the sp fp or save a
	     register should be marked as frame related.  */
	  if (op == 0)
	    RTX_FRAME_RELATED_P (insn) = 1;
	  offset -= UNITS_PER_WORD;
	}
    }
}

static void
stack_adjust (HOST_WIDE_INT amount)
{
  rtx insn;

  if (!IN_RANGE (amount, -511, 511))
    {
      /* r7 is caller saved so it can be used as a temp reg.  */
      rtx r7;
      r7 = gen_rtx_REG (word_mode, 7);
      insn = emit_move_insn (r7, GEN_INT (amount));
      if (amount < 0)
	RTX_FRAME_RELATED_P (insn) = 1;
      insn = emit_add (stack_pointer_rtx, stack_pointer_rtx, r7);
      if (amount < 0)
	RTX_FRAME_RELATED_P (insn) = 1;
    }
  else
    {
      insn = emit_add (stack_pointer_rtx,
		       stack_pointer_rtx, GEN_INT (amount));
      if (amount < 0)
	RTX_FRAME_RELATED_P (insn) = 1;
    }
}

static HOST_WIDE_INT
propeller_compute_frame_size (int size)
{
  int regno;
  HOST_WIDE_INT total_size, locals_size, args_size, pretend_size, callee_size;
  unsigned int reg_save_mask;

  locals_size = size;
  args_size = crtl->outgoing_args_size;
  pretend_size = crtl->args.pretend_args_size;
  callee_size = 0;
  reg_save_mask = 0;

  /* Build mask that actually determines which registers we save
     and calculate size required to store them in the stack.  */
  for (regno = 0; regno < PROP_FP_REGNUM; regno++)
    {
      if (df_regs_ever_live_p (regno) && !call_used_regs[regno])
	{
	  reg_save_mask |= 1 << regno;
	  callee_size += UNITS_PER_WORD;
	}
    }
  if (!(reg_save_mask & (1 << PROP_FP_REGNUM)) && frame_pointer_needed)
    {
      reg_save_mask |= 1 << PROP_FP_REGNUM;
      callee_size += UNITS_PER_WORD;
    }
  if (df_regs_ever_live_p (PROP_LR_REGNUM) || !current_function_is_leaf
      || !optimize)
    {
      reg_save_mask |= 1 << PROP_LR_REGNUM;
      callee_size += UNITS_PER_WORD;
    }

  /* Compute total frame size.  */
  total_size = pretend_size + args_size + locals_size + callee_size;

  /* Align frame to appropriate boundary.  */
  total_size = (total_size + 3) & ~3;

  /* Save computed information.  */
  current_frame_info.total_size = total_size;
  current_frame_info.callee_size = callee_size;
  current_frame_info.pretend_size = pretend_size;
  current_frame_info.locals_size = locals_size;
  current_frame_info.args_size = args_size;
  current_frame_info.reg_save_mask = reg_save_mask;

  return total_size;
}

/* Create and emit instructions for a functions prologue.  */
void
propeller_expand_prologue (void)
{
  rtx insn;

  propeller_compute_frame_size (get_frame_size ());

  if (current_frame_info.total_size > 0)
    {
      /* Add space on stack new frame.  */
      stack_adjust (-current_frame_info.total_size);

      /* Save callee save registers.  */
      if (current_frame_info.reg_save_mask != 0)
	expand_save_restore (&current_frame_info, 0);

      /* Setup frame pointer if it's needed.  */
      if (frame_pointer_needed == 1)
	{
	  /* Move sp to fp.  */
	  insn = emit_move_insn (frame_pointer_rtx, stack_pointer_rtx);
	  RTX_FRAME_RELATED_P (insn) = 1; 

	  /* Add offset - Don't use total_size, as that includes pretend_size, 
             which isn't part of this frame?  */
	  insn = emit_add (frame_pointer_rtx, 
			   frame_pointer_rtx,
			   GEN_INT (current_frame_info.args_size +
				    current_frame_info.callee_size +
				    current_frame_info.locals_size));
	  RTX_FRAME_RELATED_P (insn) = 1;
	}

      /* Prevent prologue from being scheduled into function body.  */
      emit_insn (gen_blockage ());
    }
}

/* Create an emit instructions for a functions epilogue.  */
void
propeller_expand_epilogue (void)
{
  rtx lr_rtx = gen_rtx_REG (Pmode, PROP_LR_REGNUM);

  propeller_compute_frame_size (get_frame_size ());

  if (current_frame_info.total_size > 0)
    {
      /* Prevent stack code from being reordered.  */
      emit_insn (gen_blockage ());

      /* Restore callee save registers.  */
      if (current_frame_info.reg_save_mask != 0)
	expand_save_restore (&current_frame_info, 1);

      /* Deallocate stack.  */
      stack_adjust (current_frame_info.total_size);

      /* Return to calling function.  */
      emit_jump_insn (gen_return_internal (lr_rtx));
    }
  else
    {
      /* Return to calling function.  */
      emit_jump_insn (gen_return_internal (lr_rtx));
    }
}

/* Return nonzero if this function is known to have a null epilogue.
   This allows the optimizer to omit jumps to jumps if no stack
   was created.  */

int
propeller_can_use_return (void)
{
  if (!reload_completed)
    return 0;

  if (df_regs_ever_live_p (PROP_LR_REGNUM) || crtl->profile)
    return 0;

  if (propeller_compute_frame_size (get_frame_size ()) != 0)
    return 0;

  return 1;
}


bool
propeller_legitimate_constant_p (rtx x)
{
    return true;
}

bool
propeller_const_ok_for_letter_p (HOST_WIDE_INT value, int c)
{
    switch (c)
    {
    case 'I': return value >= 0 && value <= 511;
    case 'N': return value >= -511 && value <= 0;
    default:
        gcc_unreachable();
    }
}
