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
  HOST_WIDE_INT locals_size;	/* number of bytes for local variables.  */
  HOST_WIDE_INT args_size;	/* number of bytes for outgoing arguments.  */
  HOST_WIDE_INT pretend_size;	/* number of bytes we pretend caller pushed.  */
  unsigned int reg_save_mask;	/* mask of saved registers.  */
};

/* Current frame information calculated by compute_frame_size.  */
static struct propeller_frame_info current_frame_info;
static HOST_WIDE_INT propeller_compute_frame_size (int size);
static rtx propeller_function_arg (CUMULATIVE_ARGS *cum, enum machine_mode mode,
                                   const_tree type, bool named);
static void propeller_function_arg_advance (CUMULATIVE_ARGS *cum, enum machine_mode mode,
                                const_tree type, bool named ATTRIBUTE_UNUSED);

static bool propeller_rtx_costs (rtx, int, int, int *, bool);
static int propeller_address_cost (rtx, bool);

/* register classes */
enum reg_class propeller_reg_class[FIRST_PSEUDO_REGISTER] = {
    R0_REGS, R1_REGS, GENERAL_REGS, GENERAL_REGS,
    GENERAL_REGS, GENERAL_REGS, GENERAL_REGS, GENERAL_REGS,
    GENERAL_REGS, GENERAL_REGS, GENERAL_REGS, GENERAL_REGS,
    GENERAL_REGS, GENERAL_REGS, GENERAL_REGS, GENERAL_REGS,

    GENERAL_REGS, SPECIAL_REGS, CC_REGS, SPECIAL_REGS,
    SPECIAL_REGS
};


/*
 * options handling
 */
/* Implement TARGET_OPTION_OPTIMIZATION_TABLE.  */

static const struct default_options propeller_option_optimization_table[] =
  {
    { OPT_LEVELS_1_PLUS, OPT_fomit_frame_pointer, NULL, 1 },
    { OPT_LEVELS_NONE, 0, NULL, 0 }
  };

/* Validate and override various options, and do machine dependent
 * initialization
 */
static void
propeller_option_override (void)
{
    if (flag_pic)
    {
        error ("-fPIC and -fpic are not supported");
        flag_pic = 0;
    }
    /* function CSE does not make sense for Cog mode
       we'll have to revisit this for LMM
    */
    flag_no_function_cse = 1;
}


/*
 * propeller specific attributes
 */

/* check to see if a function has a specified attribute */
static inline bool
has_func_attr (const_tree decl, const char * func_attr)
{
  tree a;

  a = lookup_attribute (func_attr, DECL_ATTRIBUTES (decl));
  return a != NULL_TREE;
}

static inline bool
is_native_function (const_tree decl)
{
  return has_func_attr (decl, "native");
}

static inline bool
is_naked_function (const_tree decl)
{
  return has_func_attr (decl, "naked");
}

/* Handle a "native" or "naked" attribute;
   arguments as in struct attribute_spec.handler.  */

static tree
propeller_handle_func_attribute (tree *node, tree name,
				      tree args ATTRIBUTE_UNUSED,
				      int flags ATTRIBUTE_UNUSED,
				      bool *no_add_attrs)
{
  if (TREE_CODE (*node) != FUNCTION_DECL)
    {
      warning (OPT_Wattributes, "%qE attribute only applies to functions",
	       name);
      *no_add_attrs = true;
    }
  return NULL_TREE;
}
/* Handle a "cogmem" attribute;
   arguments as in struct attribute_spec.handler.  */

static tree
propeller_handle_cogmem_attribute (tree *node,
				     tree name ATTRIBUTE_UNUSED,
				     tree args,
				     int flags ATTRIBUTE_UNUSED,
				     bool *no_add_attrs)
{
  if (TREE_CODE (*node) != VAR_DECL)
    {
      warning (OPT_Wattributes,
	       "%<__COGMEM__%> attribute only applies to variables");
      *no_add_attrs = true;
    }
  else if (args == NULL_TREE && TREE_CODE (*node) == VAR_DECL)
    {
      if (! (TREE_PUBLIC (*node) || TREE_STATIC (*node)))
	{
	  warning (OPT_Wattributes, "__COGMEM__ attribute not allowed "
		   "with auto storage class");
	  *no_add_attrs = true;
	}
    }

  return NULL_TREE;
}

static const struct attribute_spec propeller_attribute_table[] =
{  /* name, min_len, max_len, decl_req, type_req, fn_type_req, handler.  */
  { "naked",   0, 0, true, false,  false,  propeller_handle_func_attribute },
  { "native",  0, 0, true, false,  false,  propeller_handle_func_attribute },
  { "cogmem",  0, 0, false, false, false, propeller_handle_cogmem_attribute },
  { NULL,  0, 0, false, false, false, NULL }
};

#undef TARGET_ATTRIBUTE_TABLE
#define TARGET_ATTRIBUTE_TABLE propeller_attribute_table

static void
propeller_encode_section_info (tree decl, rtx r, int first)
{
  default_encode_section_info (decl, r, first);

   if (TREE_CODE (decl) == VAR_DECL
       && lookup_attribute ("cogmem", DECL_ATTRIBUTES (decl)))
    {
      rtx symbol = XEXP (r, 0);

      gcc_assert (GET_CODE (symbol) == SYMBOL_REF);
      SYMBOL_REF_FLAGS (symbol) |= SYMBOL_FLAG_PROPELLER_COGMEM;
    }
}

/*
 * test whether this operand is a legitimate cog memory address
 */
bool
propeller_cogaddr_p (rtx x)
{
    if (GET_CODE (x) != SYMBOL_REF) {
        return false;
    }
    if (0 != (SYMBOL_REF_FLAGS (x) & SYMBOL_FLAG_PROPELLER_COGMEM)) {
        return true;
    }
    if (CONSTANT_POOL_ADDRESS_P (x)) {
        return true;
    }
    return false;
}

/*
 * test whether this operand is a register or other reference to cog memory
 * (hence may be acted upon directly by most instructions)
 */
bool
propeller_cogmem_p (rtx op)
{
    if (register_operand (op, VOIDmode))
        return 1;

    if (GET_CODE (op) != MEM)
        return 0;

    op = XEXP (op, 0);
    if (propeller_cogaddr_p (op))
        return 1;
    return 0;
}

/*
 * functions for output of assembly language
 */
bool propeller_need_maskffffffff = false;
bool propeller_need_mask0000ffff = false;
bool propeller_need_mulsi = false;
bool propeller_need_divsi = false;
bool propeller_need_udivsi = false;
bool propeller_need_clzsi = false;
/*
 * start assembly language output
 */
static void
propeller_asm_file_start (void)
{
    if (!TARGET_OUTPUT_SPINCODE) {
        default_file_start ();
        return;
    }
    fprintf (asm_out_file, "\torg\n\n");
    fprintf (asm_out_file, "entry\n");
    /* output the prologue necessary for interfacing with spin */
    fprintf (asm_out_file, "r0\tmov\tsp,PAR\n"); 
    fprintf (asm_out_file, "r1\tmov\tr0,sp\n");
    fprintf (asm_out_file, "r2\tjmp\t#spinmain\n");
    fprintf (asm_out_file, "r3\tlong 0\n");
    fprintf (asm_out_file, "r4\tlong 0\n");
    fprintf (asm_out_file, "r5\tlong 0\n");
    fprintf (asm_out_file, "r6\tlong 0\n");
    fprintf (asm_out_file, "r7\tlong 0\n");

    fprintf (asm_out_file, "r8\tlong 0\n");
    fprintf (asm_out_file, "r9\tlong 0\n");
    fprintf (asm_out_file, "r10\tlong 0\n");
    fprintf (asm_out_file, "r11\tlong 0\n");
    fprintf (asm_out_file, "r12\tlong 0\n");
    fprintf (asm_out_file, "r13\tlong 0\n");
    fprintf (asm_out_file, "r14\tlong 0\n");
    fprintf (asm_out_file, "lr\tlong 0\n");
    fprintf (asm_out_file, "sp\tlong 0\n");
}

/*
 * end assembly language output
 */

/*
 * output code for count leading/trailing zeros
 * this is slightly tricky
 * leading zeros is converted to trailing zeros by
 * bit-reversal
 * to count trailing zeros, we remove all but the
 * lowest order 1 bit by calculating CZTMP = (x & -x);
 * then we use the basic binary search algorithm
 *   cnt = 0;
 *   if ((x & 0x0000FFFF) == 0)
 *   {   cnt += 16; x >>= 16; }
 *   if ((x & 0x000000FF) == 0)
 *   {   cnt += 8; x >>= 8; }
 *   ... etc; but since we know only one bit is set,
 *   we can skip the shifts of the value and use a shifted
 *   mask instead
 */

/* this whole thing takes 15 cycles for a "count leading zeros"
 * operation
 */
static void
pasm_clzsi(FILE *f) {
    fprintf(f, "__MASK_00FF00FF\tlong\t$00FF00FF\n");
    fprintf(f, "__MASK_0F0F0F0F\tlong\t$0F0F0F0F\n");
    fprintf(f, "__MASK_33333333\tlong\t$33333333\n");
    fprintf(f, "__MASK_55555555\tlong\t$55555555\n");
    fprintf(f, "__CLZSI\trev\tr0,#0\n");
    fprintf(f, "__CTZSI\tneg\t__TMP0,r0\n"); 
    fprintf(f, "\tand\t__TMP0,r0 wz\n");
    /* slight wrinkle: we want ctzsi(0) == 32, so start
       the count as (x == 0) ? 1 : 0
    */
    fprintf(f, "\tmov\tr0,#0\n");
    fprintf(f, "\tIF_Z\tmov\tr0,#1\n");
    fprintf(f, "\ttest\t__TMP0, __MASK_0000FFFF wz\n");
    fprintf(f, "\tIF_Z\tadd\tr0,#16\n");
    fprintf(f, "\ttest\t__TMP0, __MASK_00FF00FF wz\n");
    fprintf(f, "\tIF_Z\tadd\tr0,#8\n");
    fprintf(f, "\ttest\t__TMP0, __MASK_0F0F0F0F wz\n");
    fprintf(f, "\tIF_Z\tadd\tr0,#4\n");
    fprintf(f, "\ttest\t__TMP0, __MASK_33333333 wz\n");
    fprintf(f, "\tIF_Z\tadd\tr0,#2\n");
    fprintf(f, "\ttest\t__TMP0, __MASK_55555555 wz\n");
    fprintf(f, "\tIF_Z\tadd\tr0,#1\n");
    /* note: when calling ctz we need to use jmpret instead of
       call, since the return is named for C_CLZSI
    */
    fprintf(f, "__CLZSI_ret ret\n");
}

/*
 * unsigned division
 * input: r0 = n, r0 = d
 * output: r0 = quotient n/d, r1 = remainder
 * the algorithm used shifts d up so that it has the
 * same sign bit as n, and then counts down doing trial
 * division for as many bits as the quotient might have
 * both the shift and output bits are determined by
 * counting how many significant bits n and d have, using
 * CLZSI
 * This is significantly better than the naive division
 * algorithm when n and d are of similar sizes (since then
 * the quotient has only a few bits, so we only iterate a
 * few times). In the case where n is much bigger than d,
 * the advantage fades, but we can still use a 4 instruction
 * inner loop (instead of 5 for the naive division), which
 * cancels out the setup cost, so we don't lose anything;
 * it's a win-win!
 */

static void
pasm_udivsi(FILE *f) {
    fprintf(f, "__DIVR\tlong\t0\n");
    fprintf(f, "__DIVCNT\tlong\t0\n");

    fprintf(f, "__UDIVSI\n");
    fprintf(f, "\tmov\t__DIVR,r0\n");
    fprintf(f, "\tcall\t#__CLZSI\n");
    fprintf(f, "\tneg\t__DIVCNT,r0\n");
    fprintf(f, "\tmov\tr0,r1\n");
    fprintf(f, "\tcall\t#__CLZSI\n");
    fprintf(f, "\tadd\t__DIVCNT,r0\n");
    fprintf(f, "\tmov\tr0,#0\n");
    fprintf(f, "\tcmps\t__DIVCNT,#0 wz,wc\n");
    fprintf(f, "  IF_C\tjmp\t#__UDIVSI_done\n");
    fprintf(f, "\tshl\tr1,__DIVCNT\n");
    fprintf(f, "\tadd\t__DIVCNT,#1\n");  /* adjust for DJNZ loop */

    fprintf(f, "__UDIVSI_loop\n");
    fprintf(f, "\tcmpsub\t__DIVR,r1 wz,wc\n");
    fprintf(f, "\taddx\tr0,r0\n");
    fprintf(f, "\tshr\tr1,#1\n");
    fprintf(f, "\tdjnz\t__DIVCNT,#__UDIVSI_loop\n");

    fprintf(f, "__UDIVSI_done\n");
    fprintf(f, "\tmov\tr1,__DIVR\n");
    fprintf(f, "__UDIVSI_ret\tret\n");
}

/*
 * implement signed division by using udivsi
 */
static void
pasm_divsi(FILE *f) {
    fprintf(f, "__DIVSGN\tlong\t0\n");
    fprintf(f, "__DIVSI\tmov\t__DIVSGN,r0\n");
    fprintf(f, "\txor\t__DIVSGN,r1\n");
    fprintf(f, "\tabs\tr0,r0\n");
    fprintf(f, "\tabs\tr1,r1\n");
    fprintf(f, "\tcall\t#__UDIVSI\n");
    fprintf(f, "\tcmps\t__DIVSGN,#0 wz,wc\n");
    fprintf(f, "\tIF_B\tneg\tr0,r0\n");
    fprintf(f, "\tIF_B\tneg\tr1,r1\n");
    fprintf(f, "__DIVSI_ret\tret\n");
}

static void
propeller_asm_file_end (void)
{
    if (!TARGET_OUTPUT_SPINCODE) {
        return;
    }
    // adjust for dependencies
    if (propeller_need_divsi) {
        propeller_need_udivsi = true;
    }
    if (propeller_need_udivsi) {
        propeller_need_clzsi = true;
    }
    if (propeller_need_clzsi) {
        propeller_need_mask0000ffff = true;
    }
    if (propeller_need_mask0000ffff) {
        fprintf(asm_out_file, "__MASK_0000FFFF\tlong\t$0000FFFF\n");
    }
    if (propeller_need_maskffffffff) {
        fprintf(asm_out_file, "__MASK_FFFFFFFF\tlong\t$FFFFFFFF\n");
    }
    if (propeller_need_mulsi || propeller_need_clzsi) {
        fprintf(asm_out_file, "__TMP0\tlong\t0\n");
    }
    if (propeller_need_mulsi) {
        fprintf(asm_out_file, "__MULSI\n");
        fprintf(asm_out_file, "\tmov\t__TMP0,r0\n");
        fprintf(asm_out_file, "\tmin\t__TMP0,r1\n");
        fprintf(asm_out_file, "\tmax\tr1,r0\n");
        fprintf(asm_out_file, "\tmov\tr0,#0\n");
        fprintf(asm_out_file, ":mloop\n");
        fprintf(asm_out_file, "\tshr\tr1,#1 wz,wc\n");
        fprintf(asm_out_file, "  IF_C\tadd\tr0,__TMP0\n");
        fprintf(asm_out_file, "\tadd\t__TMP0,__TMP0\n");
        fprintf(asm_out_file, "  IF_NZ\tjmp\t#:mloop\n");
        fprintf(asm_out_file, "__MULSI_ret\tret\n");
    }
    if (propeller_need_clzsi) {
        pasm_clzsi(asm_out_file);
    }
    if (propeller_need_udivsi) {
        pasm_udivsi(asm_out_file);
    }
    if (propeller_need_divsi) {
        pasm_divsi(asm_out_file);
    }
}

/*
 * output a label
 */
void
propeller_output_label(FILE *file, const char * label)
{
    assemble_name (file, label);
    fputs("\n", file);
}

/* The purpose of this function is to override the default behavior of
   BSS objects.  Normally, they go into .bss or .sbss via ".common"
   directives, but we need to override that if they are for cog memory
*/

void
propeller_asm_output_aligned_common (FILE *stream,
				     tree decl,
				     const char *name,
				     int size,
				     int align,
				     int global)
{
  rtx mem = decl == NULL_TREE ? NULL_RTX : DECL_RTL (decl);
  rtx symbol;

  if (mem != NULL_RTX
      && MEM_P (mem)
      && GET_CODE (symbol = XEXP (mem, 0)) == SYMBOL_REF
      && SYMBOL_REF_FLAGS (symbol) & SYMBOL_FLAG_PROPELLER_COGMEM)
    {
      const char *name2;
      int i;

      name2 = default_strip_name_encoding (name);
      fprintf(stream, "%s\n", name2);
      for (i = 0; i < size; i+=4) 
          fprintf (stream, "\tlong\t0\n");
      return;
    }

  if (!global)
    {
      fprintf (stream, "\t.local\t");
      assemble_name (stream, name);
      fprintf (stream, "\n");
    }
  fprintf (stream, "\t.comm\t");
  assemble_name (stream, name);
  fprintf (stream, ",%u,%u\n", size, align / BITS_PER_UNIT);
}


/* test whether punctuation is valid in operand output */
bool
propeller_print_operand_punct_valid_p (unsigned char code)
{
    return false;
}

/* The PRINT_OPERAND worker; prints an operand to an assembler
 * instruction.
 * Our specific ones:
 *   J   Select a predicate for a conditional execution
 *   j   Select the inverse predicate for a conditional execution
 */

#define LETTERJ(YES, REV)  (letter == 'J') ? (YES) : (REV)

void
propeller_print_operand (FILE * file, rtx op, int letter)
{
  enum rtx_code code;
  const char *str;

  code = GET_CODE (op);
  if (letter == 'J' || letter == 'j') {
      switch (code) {
      case NE:
          str = LETTERJ("NE", "E ");
          break;
      case EQ:
          str = LETTERJ("E ", "NE");
          break;
      case LT:
      case LTU:
          str = LETTERJ("B ", "AE");
          break;
      case GE:
      case GEU:
          str = LETTERJ("AE", "B ");
          break;
      case GT:
      case GTU:
          str = LETTERJ("A ", "BE");
          break;
      case LE:
      case LEU:
          str = LETTERJ("BE", "A ");
          break;
      default:
          output_operand_lossage("invalid mode for %%J");
          return;
      }
      fprintf (file, "IF_%s", str);
      return;
  }

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
  else if (code == CONST_DOUBLE)
    {
      if ((CONST_DOUBLE_LOW (op) != 0) || (CONST_DOUBLE_HIGH (op) != 0))
	output_operand_lossage ("only 0.0 can be loaded as an immediate");
      else
	fprintf (file, "#0");
    }
  else if (code == EQ)
    fprintf (file, "E  ");
  else if (code == NE)
    fprintf (file, "NE ");
  else if (code == GT || code == GTU)
    fprintf (file, "A  ");
  else if (code == LT || code == LTU)
    fprintf (file, "B  ");
  else if (code == GE || code == GEU)
    fprintf (file, "AE ");
  else if (code == LE || code == LEU)
    fprintf (file, "BE ");
  else
    {
       if (code == CONST_INT) fprintf (file, "#");
       output_addr_const (file, op);
    }
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
 * determine whether it's OK to store a value of mode MODE into a register
 */
bool
propeller_hard_regno_mode_ok (unsigned int reg, enum machine_mode mode)
{
    /* condition codes only go in the CC register, and it can only
       hold condition codes
    */
    if (mode == CCmode || mode == CCUNSmode) {
        return reg == PROP_CC_REGNUM;
    }
    if (reg == PROP_CC_REGNUM) {
        return false;
    }
    return true;
}

/*
 * true if a value in mode A is accessible in mode B without copying
 */
bool
propeller_modes_tieable_p (enum machine_mode A ATTRIBUTE_UNUSED, enum machine_mode B ATTRIBUTE_UNUSED)
{
    return true;
}


/*
 * check for legitimate addresses
 */
bool
propeller_legitimate_address_p (enum machine_mode mode ATTRIBUTE_UNUSED, rtx addr, bool strict)
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

    /* allow cog memory references */
    if (propeller_cogaddr_p (addr))
        return true;
    if (GET_CODE (addr) == LABEL_REF)
        return true;

    return false;
}


/* costs for various operations */

/* Implement the TARGET_RTX_COSTS hook.

   Speed-relative costs are relative to COSTS_N_INSNS, which is intended
   to represent cycles.  Size-relative costs are in bytes.  */
static bool
propeller_rtx_costs (rtx x, int code, int outer_code ATTRIBUTE_UNUSED, int *total_ptr, bool speed)
{
    int total;
    bool done = false;

    switch (code) {
    case CONST_INT:
        if (!speed) {
            HOST_WIDE_INT ival = INTVAL (x);
            if (ival >= -511 && ival < 511)
                total = 0;
            else
                total = (speed ? COSTS_N_INSNS(1) : 4);
            done = true;
            break;
        }
        /* fall through */
    case CONST:
    case LABEL_REF:
    case SYMBOL_REF:
    case CONST_DOUBLE:
        total = (speed ? COSTS_N_INSNS(1) : 4);
        done = true;
        break;
    case PLUS:
    case MINUS:
    case AND:
    case IOR:
    case XOR:
    case NOT:
    case NEG:
    case COMPARE:
    case ASHIFT:
    case ASHIFTRT:
    case LSHIFTRT:
        total = (speed ? COSTS_N_INSNS(1) : 4);
        break;

    case MULT:
    case DIV:
    case UDIV:
    case MOD:
    case UMOD:
        total = (speed ? COSTS_N_INSNS(16) : 8);
        done = true;
        break;

    case MEM:
        total = propeller_address_cost (XEXP (x, 0), speed);
        total = speed ? COSTS_N_INSNS (2 + total) : total;
        done = true;
        break;
    default:
        /* assume external call */
        total = (speed ? COSTS_N_INSNS(10) : 8);
        break;
    }
    *total_ptr = total;
    return done;
}


static int
propeller_address_cost (rtx addr, bool speed)
{
  int total;
  rtx a, b;

  if (GET_CODE (addr) != PLUS) {
    total = 1;
  } else {
    a = XEXP (addr, 0);
    b = XEXP (addr, 1);
    if (REG_P (a) && REG_P (b)) {
        /* try to discourage REG+REG addressing */
        total = 4;
    } else {
        total = 2;
    }
  }
  return speed ? COSTS_N_INSNS(total) : 4*total;
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


/* Convert from bytes to ints.  */
#define PROP_NUM_INTS(X) (((X) + UNITS_PER_WORD - 1) / UNITS_PER_WORD)

/* The number of (integer) registers required to hold a quantity of
   TYPE MODE.  */
#define PROP_NUM_REGS(MODE, TYPE)                       \
  PROP_NUM_INTS ((MODE) == BLKmode ?                     \
  int_size_in_bytes (TYPE) : GET_MODE_SIZE (MODE))

/* function parameter related functions */
/* Determine where to put an argument to a function.
   Value is zero to push the argument on the stack,
   or a hard register in which to store the argument.

   MODE is the argument's machine mode.
   TYPE is the data type of the argument (as a tree).
    This is null for libcalls where that information may
    not be available.
   CUM is a variable of type CUMULATIVE_ARGS which gives info about
    the preceding args and about the function being called.
   NAMED is nonzero if this argument is a named parameter
    (otherwise it is an extra parameter matching an ellipsis).  */

static rtx
propeller_function_arg (CUMULATIVE_ARGS *cum, enum machine_mode mode,
		   const_tree type, bool named)
{
  if (mode == VOIDmode)
    /* Compute operand 2 of the call insn.  */
    return GEN_INT (0);

  if (targetm.calls.must_pass_in_stack (mode, type))
    return NULL_RTX;

  if (!named || (*cum + PROP_NUM_REGS (mode, type) > NUM_ARG_REGS))
    return NULL_RTX;

  return gen_rtx_REG (mode, *cum + PROP_R0);
}

static void
propeller_function_arg_advance (CUMULATIVE_ARGS *cum, enum machine_mode mode,
			   const_tree type, bool named ATTRIBUTE_UNUSED)
{
  *cum += PROP_NUM_REGS (mode, type);
}


/*
 * comparison functions
 */
/* SELECT_CC_MODE.  */

enum machine_mode
propeller_select_cc_mode (RTX_CODE op, rtx x ATTRIBUTE_UNUSED, rtx y ATTRIBUTE_UNUSED)
{
  if (op == EQ || op == NE)
    return CC_Zmode;
  if (op == GTU || op == LTU || op == GEU || op == LEU)
    return CCUNSmode;

  return CCmode;
}

/*
 * generate CC_REG in appropriate mode
 */
rtx
propeller_gen_compare_reg (RTX_CODE code, rtx x, rtx y)
{
    enum machine_mode ccmode = SELECT_CC_MODE (code, x, y);
    return gen_rtx_REG (ccmode, PROP_CC_REGNUM);
}

/*
 * return TRUE or FALSE depending on whether the first SET
 * in insn has source and destination with matching CC modes, and
 * that the CC mode is at least as constrained as req_mode
 */
bool
propeller_match_ccmode (rtx insn, enum machine_mode req_mode)
{
  rtx op1, flags;
  enum machine_mode flags_mode;

  if (req_mode == VOIDmode)
    return false;

  gcc_checking_assert (XVECLEN (PATTERN (insn), 0) == 2);

  op1 = XVECEXP (PATTERN (insn), 0, 1);
  gcc_checking_assert (GET_CODE (SET_SRC (op1)) == COMPARE);

  flags = SET_DEST (op1);
  flags_mode = GET_MODE (flags);

  if (GET_MODE (SET_SRC (op1)) != flags_mode)
    return false;
  if (GET_MODE_CLASS (flags_mode) != MODE_CC)
    return false;

  /* Ensure that the mode of FLAGS is compatible with REQ_MODE.
   * For example, CC_Z is compatible with both CCUNS and CC,
   * but not necessarily vice-versa
   */
  switch (flags_mode)
  {
  case CC_Zmode:
      return (req_mode == CC_Zmode || req_mode == CCmode || req_mode == CCUNSmode);
  default:
      return (req_mode == flags_mode);
  }
}


/*
 * stack related functions
 */

/* Typical stack layout should looks like this after the function's prologue:

                            |    |
                              --                       ^
                            |    | \                   |
                            |    |   arguments saved   | Increasing
                            |    |   on the stack      |  addresses
    PARENT   arg pointer -> |    | /
  -------------------------- ---- -------------------
    CHILD                   |    | \
                            |    |   call saved
                            |    |   registers
        hard fp ->          |    | /
                              --
                            |    | \
                            |    |   local
                            |    |   variables
        frame pointer ->    |    | /
                              --
                            |    | \
                            |    |   outgoing          | Decreasing
                            |    |   arguments         |  addresses
   current stack pointer -> |    | /                   |
  -------------------------- ---- ------------------   V
                            |    |                 */

HOST_WIDE_INT
propeller_initial_elimination_offset (int from, int to)
{
  HOST_WIDE_INT offset;
  HOST_WIDE_INT base;

  /* the -UNITS_PER_WORD in stack pointer offsets is
     because we predecrement the stack pointer before using it,
   */
  base = propeller_compute_frame_size (get_frame_size ()) - UNITS_PER_WORD;


  if (from == ARG_POINTER_REGNUM)
  {
      if (to == STACK_POINTER_REGNUM)
          offset = base;
      else if (to == HARD_FRAME_POINTER_REGNUM)
          offset = current_frame_info.callee_size;
      else
          gcc_unreachable ();
  }
  else if (from == FRAME_POINTER_REGNUM)
  {
      if (to == STACK_POINTER_REGNUM)
          offset = base - (current_frame_info.callee_size + current_frame_info.locals_size);
      else if (to == HARD_FRAME_POINTER_REGNUM)
          offset = -(current_frame_info.locals_size+UNITS_PER_WORD);
      else
          gcc_unreachable ();
  }
  else
      gcc_unreachable();
  return offset;
}
/* Generate and emit RTL to save callee save registers.  */
/* returns total number of bytes pushed on stack */
static int
expand_save_registers (struct propeller_frame_info *info)
{
  unsigned int reg_save_mask = info->reg_save_mask;
  int regno;
  int pushed = 0;
  rtx insn;
  rtx sp;

  sp = gen_rtx_REG (word_mode, PROP_SP_REGNUM);

  /* Callee saves are below locals and above outgoing arguments.  */
  /* push registers from low to high (so r15 gets pushed last, and
     hence is available early for us to use in the epilogue) */
  for (regno = 0; regno <= 15; regno++)
    {
      if ((reg_save_mask & (1 << regno)) != 0)
      {
          rtx mem;
          insn = emit_add (sp, sp, GEN_INT(-4));
          RTX_FRAME_RELATED_P (insn) = 1;
          mem = gen_rtx_MEM (word_mode, sp);
          insn = emit_move_insn (mem, gen_rtx_REG (word_mode, regno));
          RTX_FRAME_RELATED_P (insn) = 1;
          pushed += UNITS_PER_WORD;
      }
    }
  return pushed;
}

/* Generate and emit RTL to restore callee save registers.  */
static void
expand_restore_registers (struct propeller_frame_info *info)
{
  unsigned int reg_save_mask = info->reg_save_mask;
  int regno;
  rtx insn;
  rtx sp;

  sp = gen_rtx_REG (word_mode, PROP_SP_REGNUM);

  /* Callee saves are below locals and above outgoing arguments.  */
  /* we pushed registers from high to low (so r0 gets pushed first),
   * so restore in the opposite order
   */
  for (regno = 15; regno >= 0; regno--)
    {
      if ((reg_save_mask & (1 << regno)) != 0)
      {
          rtx mem;
          mem = gen_rtx_MEM (word_mode, sp);
          insn = emit_move_insn (gen_rtx_REG (word_mode, regno), mem);
          insn = emit_add (sp, sp, GEN_INT(4));
      }
    }
}

static void
stack_adjust (HOST_WIDE_INT amount)
{
  rtx insn;

  if (amount == 0) return;
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

static bool
propeller_use_frame_pointer(void)
{
    return frame_pointer_needed;
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
  if (!(reg_save_mask & (1 << PROP_FP_REGNUM)) && propeller_use_frame_pointer())
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

/* set to 1 to keep scheduling from moving around stuff in the prologue
 * and epilogue
 */
#define PROLOGUE_BLOCKAGE 1

/* Create and emit instructions for a functions prologue.  */
void
propeller_expand_prologue (void)
{
  rtx insn;
  rtx hardfp;
  int pushed = 0;

  /* naked functions have no prologue or epilogue */
  if (is_naked_function (current_function_decl))
    return;

  propeller_compute_frame_size (get_frame_size ());

  if (current_frame_info.total_size > 0)
    {
      /* Save callee save registers.  */
      if (current_frame_info.reg_save_mask != 0)
          pushed = expand_save_registers (&current_frame_info);

      /* Setup frame pointer if it's needed.  */
      if (propeller_use_frame_pointer())
      {
	  /* Move sp to fp.  */
          hardfp = gen_rtx_REG (Pmode, PROP_FP_REGNUM);
          insn = emit_move_insn (hardfp, stack_pointer_rtx);
          RTX_FRAME_RELATED_P (insn) = 1; 
      }

      /* Add space on stack new frame.  */
      stack_adjust (-(current_frame_info.total_size - pushed));


#ifdef PROLOGUE_BLOCKAGE
      /* Prevent prologue from being scheduled into function body.  */
      emit_insn (gen_blockage ());
#endif
    }
}

/* Create an emit instructions for a functions epilogue.  */
void
propeller_expand_epilogue (bool is_sibcall)
{
  rtx lr_rtx = gen_rtx_REG (Pmode, PROP_LR_REGNUM);

  if (is_naked_function (current_function_decl))
  {
      gcc_assert(! is_sibcall);
      /* Naked functions use their own, programmer provided epilogues.
         But, in order to keep gcc happy we have to generate some kind of
         epilogue RTL.  */
      emit_jump_insn (gen_naked_return ());
      return;

  }
  propeller_compute_frame_size (get_frame_size ());

  if (current_frame_info.total_size > 0)
    {
#ifdef PROLOGUE_BLOCKAGE
      /* Prevent stack code from being reordered.  */
      emit_insn (gen_blockage ());
#endif

      /* Deallocate stack.  */
      if (propeller_use_frame_pointer ())
      {
          /* the hardware frame pointer points just below the saved registers,
             so we can get back there by moving it to the sp
          */
          rtx hardfp_rtx = gen_rtx_REG (Pmode, PROP_FP_REGNUM);
          emit_move_insn (stack_pointer_rtx, hardfp_rtx);
      }
      else
      {
          stack_adjust (current_frame_info.total_size - current_frame_info.callee_size);
      }
      /* Restore callee save registers.  */
      if (current_frame_info.reg_save_mask != 0)
          expand_restore_registers (&current_frame_info);

      }

  /* Return to calling function.  */
  emit_jump_insn (gen_return_internal (lr_rtx));
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

/*
 * support functions for moving stuff
 */


bool
propeller_legitimate_constant_p (rtx x)
{
    switch (GET_CODE (x))
    {
    case LABEL_REF:
        return true;
    case CONST_DOUBLE:
        if (GET_MODE (x) == VOIDmode)
            return true;
        return false;
    case CONST:
    case SYMBOL_REF:
    case CONST_VECTOR:
        return false;
    case CONST_INT:
        return (INTVAL (x) >= -511 && INTVAL (x) <= 511);
    default:
        return true;
    }
}
bool
propeller_const_ok_for_letter_p (HOST_WIDE_INT value, int c)
{
    switch (c)
    {
    case 'I': return value >= 0 && value <= 511;
    case 'N': return value >= -511 && value <= 0;
    case 'W': return value < 0 && value > 511;
    default:
        gcc_unreachable();
    }
}



#undef TARGET_OPTION_OVERRIDE
#define TARGET_OPTION_OVERRIDE propeller_option_override
#undef TARGET_OPTION_OPTIMIZATION_TABLE
#define TARGET_OPTION_OPTIMIZATION_TABLE propeller_option_optimization_table

#undef TARGET_ASM_FILE_START
#define TARGET_ASM_FILE_START propeller_asm_file_start
#undef TARGET_ASM_FILE_END
#define TARGET_ASM_FILE_END propeller_asm_file_end

#undef TARGET_PRINT_OPERAND
#define TARGET_PRINT_OPERAND propeller_print_operand
#undef TARGET_PRINT_OPERAND_ADDRESS
#define TARGET_PRINT_OPERAND_ADDRESS propeller_print_operand_address
#undef TARGET_PRINT_OPERAND_PUNCT_VALID_P
#define TARGET_PRINT_OPERAND_PUNCT_VALID_P propeller_print_operand_punct_valid_p
#undef TARGET_PROMOTE_FUNCTION_MODE
#define TARGET_PROMOTE_FUNCTION_MODE default_promote_function_mode_always_promote
#undef TARGET_FUNCTION_ARG
#define TARGET_FUNCTION_ARG propeller_function_arg
#undef TARGET_FUNCTION_ARG_ADVANCE
#define TARGET_FUNCTION_ARG_ADVANCE propeller_function_arg_advance
#undef TARGET_PROMOTE_PROTOTYPES
#define TARGET_PROMOTE_PROTOTYPES hook_bool_const_tree_false
#undef TARGET_LEGITIMATE_ADDRESS_P
#define TARGET_LEGITIMATE_ADDRESS_P propeller_legitimate_address_p
#undef  TARGET_RTX_COSTS
#define TARGET_RTX_COSTS propeller_rtx_costs
#undef  TARGET_ADDRESS_COST
#define TARGET_ADDRESS_COST propeller_address_cost
#undef  TARGET_ENCODE_SECTION_INFO
#define TARGET_ENCODE_SECTION_INFO propeller_encode_section_info

#undef TARGET_ASM_BYTE_OP
#define TARGET_ASM_BYTE_OP "\tbyte\t"
#undef TARGET_ASM_ALIGNED_HI_OP
#define TARGET_ASM_ALIGNED_HI_OP "\tword\t"
#undef TARGET_ASM_UNALIGNED_HI_OP
#define TARGET_ASM_UNALIGNED_HI_OP "\tword\t"
#undef TARGET_ASM_ALIGNED_SI_OP
#define TARGET_ASM_ALIGNED_SI_OP "\tlong\t"
#undef TARGET_ASM_UNALIGNED_SI_OP
#define TARGET_ASM_UNALIGNED_SI_OP "\tlong\t"

struct gcc_target targetm = TARGET_INITIALIZER;
