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
#include "obstack.h"
#include "tree.h"
#include "c-tree.h"
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
#include "diagnostic-core.h"
#include "tree-pass.h"

/*
 * define USE_HUBCOG_DIRECTIVES to 1 to get .hub_ram and .cog_ram put into
 * the output sections in Cog mode
 */

#define USE_HUBCOG_DIRECTIVES 0



/*
 * frame setup
 */
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

/* sets various assembler strings depending on options */
const char *propeller_text_asm_op;
const char *propeller_data_asm_op;
const char *propeller_bss_asm_op;

static const char *hex_prefix;


/* Implement TARGET_OPTION_OPTIMIZATION_TABLE.  */

static const struct default_options propeller_option_optimization_table[] =
  {
    /* turn this on to disable frame pointer by default */
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

    /* -mxmm implies -mlmm */
    if (TARGET_XMM)
      target_flags |= MASK_LMM;

    /* function CSE does not make sense for Cog mode
       we'll have to revisit this for LMM
    */
    flag_no_function_cse = 1;

    /*
     * set up target specific flags
     */
    if (TARGET_OUTPUT_SPINCODE)
      target_flags |= MASK_PASM;

    hex_prefix = "0x"; /* default to gas syntax */
    /* set up the assembly output */
    if (TARGET_PASM)
      {
	propeller_text_asm_op = "\t'.text";
	propeller_data_asm_op = "\t'.data";
	propeller_bss_asm_op = "\t'.section\t.bss";
	hex_prefix = "$";
      }
    else if (TARGET_LMM || !USE_HUBCOG_DIRECTIVES)
      {
	propeller_text_asm_op = "\t.text";
	propeller_data_asm_op = "\t.data";
	propeller_bss_asm_op = "\t.section\t.bss";
      }
    else
      {
	propeller_text_asm_op = "\t.text\n\t.cog_ram";
	propeller_data_asm_op = "\t.data\n\t.hub_ram";
	propeller_bss_asm_op = "\t.section\t.bss\n\t.hub_ram";
      }
}


/*
 * propeller specific attributes
 */

/* check to see if a function has a specified attribute */
static inline bool
has_func_attr (tree decl, const char * func_attr)
{
  tree attrs, a;

  if (!decl) return false;
  attrs = DECL_ATTRIBUTES (decl);
  a = lookup_attribute (func_attr, attrs);
  return a != NULL_TREE;
}

static inline bool
is_native_function (tree decl)
{
  return has_func_attr (decl, "native");
}

static inline bool
is_naked_function (tree decl)
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
  tree decl = *node;

  if (TYPE_MODE (TREE_TYPE (decl)) == BLKmode)
    {
        error ("data type of %q+D isn%'t suitable for a register",
               decl);
        *no_add_attrs = true;
        return NULL_TREE;
    }

  if (TREE_CODE (decl) != VAR_DECL)
    {
      warning (OPT_Wattributes,
	       "%<__COGMEM__%> attribute only applies to variables");
      *no_add_attrs = true;
    }
  else if (args == NULL_TREE && TREE_CODE (decl) == VAR_DECL)
    {
      if (! (TREE_PUBLIC (*node) || TREE_STATIC (*node)))
	{
	  warning (OPT_Wattributes, "__COGMEM__ attribute not allowed "
		   "with auto storage class");
	  *no_add_attrs = true;
	}
    }
  
  if (*no_add_attrs == false)
    {
        C_DECL_REGISTER (decl) = 1;
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
    if (CONSTANT_POOL_ADDRESS_P (x) && !TARGET_LMM) {
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
	if (TARGET_LMM)
	  fprintf (asm_out_file, "\t.hub_ram\n");
        return;
    }
    fprintf (asm_out_file, "\torg\n\n");
    fprintf (asm_out_file, "entry\n");
    /* output the prologue necessary for interfacing with spin */
    fprintf (asm_out_file, "r0\tmov\tsp,PAR\n"); 
    fprintf (asm_out_file, "r1\tmov\tr0,sp\n");
    fprintf (asm_out_file, "r2\tjmp\t#_spinmain\n");
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
  fprintf(f, "__MASK_00FF00FF\tlong\t%s00FF00FF\n", hex_prefix);
  fprintf(f, "__MASK_0F0F0F0F\tlong\t%s0F0F0F0F\n", hex_prefix);
  fprintf(f, "__MASK_33333333\tlong\t%s33333333\n", hex_prefix);
  fprintf(f, "__MASK_55555555\tlong\t%s55555555\n", hex_prefix);
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
  /* adjust for dependencies */
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
    fprintf(asm_out_file, "__MASK_0000FFFF\tlong\t%s0000FFFF\n", hex_prefix);
  }
  if (propeller_need_maskffffffff) {
    fprintf(asm_out_file, "__MASK_FFFFFFFF\tlong\t%sFFFFFFFF\n", hex_prefix);
  }
  if (propeller_need_mulsi || propeller_need_clzsi) {
    fprintf(asm_out_file, "__TMP0\tlong\t0\n");
  }
  if (propeller_need_mulsi)
    {
      fprintf(asm_out_file, "__MULSI\n");
      fprintf(asm_out_file, "\tmov\t__TMP0,r0\n");
      fprintf(asm_out_file, "\tmin\t__TMP0,r1\n");
      fprintf(asm_out_file, "\tmax\tr1,r0\n");
      fprintf(asm_out_file, "\tmov\tr0,#0\n");
      fprintf(asm_out_file, "__MULSI_loop\n");
      fprintf(asm_out_file, "\tshr\tr1,#1 wz,wc\n");
      fprintf(asm_out_file, "  IF_C\tadd\tr0,__TMP0\n");
      fprintf(asm_out_file, "\tadd\t__TMP0,__TMP0\n");
      fprintf(asm_out_file, "  IF_NZ\tjmp\t#__MULSI_loop\n");
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
      int i;

      assemble_name (stream, name);
      fprintf(stream, "\n");
      for (i = 0; i < size; i+=4) 
          fprintf (stream, "\tlong\t0\n");
      return;
    }

  if (!global && !TARGET_PASM)
    {
      fprintf (stream, "\t.local\t");
      assemble_name (stream, name);
      fprintf (stream, "\n");
    }
  if (TARGET_PASM)
    {
      int i = size;
      assemble_name (stream, name);
      fprintf (stream, "\n");
      while (i > 4) {
	fprintf (stream, "\tlong\t0\n");
	i -= 4;
      }
      while (i > 0) {
	fprintf (stream, "\tbyte\t0\n");
	i--;
      }
    }
  else
    {
      fprintf (stream, "\t.comm\t");
      assemble_name (stream, name);
      if (align)
	fprintf (stream, ",%u,%u\n", size, align / BITS_PER_UNIT);
      else
	fprintf (stream, ",%u\n", size);
    }
}

/* test whether punctuation is valid in operand output */
bool
propeller_print_operand_punct_valid_p (unsigned char code ATTRIBUTE_UNUSED)
{
    return false;
}

/* The PRINT_OPERAND worker; prints an operand to an assembler
 * instruction.
 * Our specific ones:
 *   p   Select a predicate for a conditional execution
 *   P   Select the inverse predicate for a conditional execution
 *   M   Print the complement of a constant integer
 *   m   Print a mask (1<<n)-1 where n is a constant
 *   B   Print a mask (1<<n) where n is a constant
 */

#define PREDLETTER(YES, REV)  (letter == 'p') ? (YES) : (REV)

void
propeller_print_operand (FILE * file, rtx op, int letter)
{
  enum rtx_code code;
  const char *str;

  code = GET_CODE (op);
  if (letter == 'p' || letter == 'P') {
      switch (code) {
      case NE:
          str = PREDLETTER("NE", "E ");
          break;
      case EQ:
          str = PREDLETTER("E ", "NE");
          break;
      case LT:
      case LTU:
          str = PREDLETTER("B ", "AE");
          break;
      case GE:
      case GEU:
          str = PREDLETTER("AE", "B ");
          break;
      case GT:
      case GTU:
          str = PREDLETTER("A ", "BE");
          break;
      case LE:
      case LEU:
          str = PREDLETTER("BE", "A ");
          break;
      default:
          gcc_unreachable ();
      }
      fprintf (file, "IF_%s", str);
      return;
  }
  if (letter == 'M') {
      if (code != CONST_INT) {
          gcc_unreachable ();
      }
      fprintf (file, "#%s%lx", hex_prefix, ~INTVAL (op));
      return;
  }
  if (letter == 'm') {
      if (code != CONST_INT) {
          gcc_unreachable ();
      }
      fprintf (file, "#%s%lx", hex_prefix, (1L<<INTVAL (op))-1);
      return;
  }
  if (letter == 'B') {
      if (code != CONST_INT) {
          gcc_unreachable ();
      }
      fprintf (file, "#%s%lx", hex_prefix, (1L<<INTVAL (op)));
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
    if (GET_MODE_CLASS (mode) == MODE_CC) {
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
    /* the only kinds of address the propeller 1 supports is register indirect
       and cog memory indirect */
    while (GET_CODE (addr) == SUBREG)
        addr = SUBREG_REG (addr);
    if (GET_CODE (addr) == REG) {
        int regno = REGNO(addr);
        if (!HARD_REGISTER_NUM_P (regno)) {
            if (!strict) return true;
            regno = reg_renumber[regno];
        }
        if (regno == ARG_POINTER_REGNUM || regno == FRAME_POINTER_REGNUM)
            return true;

        if (HARD_REGNO_OK_FOR_BASE_P(regno))
            return true;

        return false;
    }

    /* allow cog memory references */
    if (propeller_cogaddr_p (addr))
        return true;
    if (GET_CODE (addr) == LABEL_REF)
        return true;

    /* allow cog memory indirect */
    if (propeller_cogmem_p (addr))
        return true;

    return false;
}


/* costs for various operations */

/* Implement the TARGET_RTX_COSTS hook.

   Speed-relative costs are relative to COSTS_N_INSNS, which is intended
   to represent cycles.  Size-relative costs are in words.  */
static bool
propeller_rtx_costs (rtx x, int code, int outer_code ATTRIBUTE_UNUSED, int *total_ptr, bool speed)
{
    int total = *total_ptr;
    bool done = false;

    switch (code) {
    case CONST_INT:
        {
            HOST_WIDE_INT ival = INTVAL (x);
            if (ival >= -511 && ival < 511)
                total = 0;
            else
                total = (speed ? COSTS_N_INSNS(1) : 1);
            done = true;
        }
        break;
    case CONST:
    case LABEL_REF:
    case SYMBOL_REF:
    case CONST_DOUBLE:
        total = (speed ? COSTS_N_INSNS(1) : 1);
        done = true;
        break;
    case PLUS:
    case MINUS:
    case AND:
    case IOR:
    case XOR:
    case NOT:
    case NEG:
    case ABS:
    case COMPARE:
    case ASHIFT:
    case ASHIFTRT:
    case LSHIFTRT:
    case ZERO_EXTRACT:
        total = (speed ? COSTS_N_INSNS(1) : 1);
        break;

    case MULT:
        total = (speed ? COSTS_N_INSNS(16) : 2);
        break;

    case DIV:
    case UDIV:
    case MOD:
    case UMOD:
        total = (speed ? COSTS_N_INSNS(100) : 2);
        done = true;
        break;

    case MEM:
        total = propeller_address_cost (XEXP (x, 0), speed);
        done = true;
        break;

    default:
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
    /* cog memory addresses are free */
    total = propeller_cogaddr_p (addr)  ? 0 : 1;
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
  return speed ? COSTS_N_INSNS(total) : total;
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
  /* for unsigned ltu and geu only the carry matters */
  if (op == LTU || op == GEU)
    return CC_Cmode;
  if (op == GTU || op == LEU)
    return CCUNSmode;

  return CCmode;
}

/*
 * canonicalize a comparison so that we are more likely to recognize
 * it, and it produces better code
 * Generally on the Propeller we prefer to have conditions that just
 * rely on either the carry or the zero flag (not combinations of them),
 * since there are some additional predicable instructions that use these.
 */
enum rtx_code
propeller_canonicalize_comparison (enum rtx_code code, rtx *op0, rtx *op1)
{
  enum machine_mode mode;
  unsigned HOST_WIDE_INT i, maxval;
  rtx temp;

  mode = GET_MODE (*op0);
  if (mode == VOIDmode)
    mode = GET_MODE (*op1);

  /* change "< 512" to "<= 511 */
  if ( ((code == LT) || (code == LTU))
       && GET_CODE (*op1) == CONST_INT
       && INTVAL (*op1) == 512
       && SCALAR_INT_MODE_P (GET_MODE (*op0)) )
    {
      *op1 = GEN_INT (511);
      return (code == LT) ? LE : LEU;
    }

  maxval = 511;

  /* if op1 is not a constant we can swap the comparison */
  /* do so if it will turn GT to LT or LE to GE  (LT and GE are our
     preferred forms)
   */
  if ( (code == GTU || code == LEU || code == GT || code == LE)
       && (mode == SImode) )
    {
      /* first try to use the better comparison in place
       * e.g. x > N => x >= N+1
       */
      if ( GET_CODE (*op1) == CONST_INT )
	{
	  i = INTVAL (*op1);
	  switch (code)
	    {
	    case GT:
	    case LE:
	      if (i != maxval)
		{
		  *op1 = GEN_INT (i+1);
		  return code == GT ? GE : LT;
		}
	      break;
	    case GTU:
	    case LEU:
	      if (i != maxval)
		{
		  *op1 = GEN_INT (i+1);
		  return code == GTU ? GEU : LTU;
		}
	      break;
	    default:
	      gcc_unreachable ();
	    }
	  /* give up if it was a constant integer, better to just
	     compare and accept that some optimization opportunities are
	     missed */
	  return code;
	}
      /* otherwise, reverse the condition */
      temp = *op0;
      *op0 = *op1;
      *op1 = temp;
      return swap_condition (code);
    }
       
  return code;
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
 * return true if mode flags_mode is compatible with req_mode
 * for example, CC_Z is compatible with both CCUNS and CC,
 * but not necessarily vice-versa (CC_Z only guarantees that the
 * Z bit is set correctly
 */
static bool
ccmodes_compatible(enum machine_mode flags_mode, enum machine_mode req_mode)
{
  switch (flags_mode)
    {
    case CC_Zmode:
      return (req_mode == CC_Zmode || req_mode == CCmode || req_mode == CCUNSmode);
    case CC_Cmode:
      return (req_mode == CC_Cmode || req_mode == CCUNSmode);
    default:
      return (req_mode == flags_mode);
    }
}

/* Return true if SET either doesn't set the CC register, or else
   the source and destination have matching CC modes and that
   CC mode is at least as constrained as REQ_MODE.  */

static bool
propeller_match_ccmode_set (rtx set, enum machine_mode req_mode)
{
  enum machine_mode set_mode;

  gcc_assert (GET_CODE (set) == SET);

  if (GET_CODE (SET_DEST (set)) != REG || (CC_REG != REGNO (SET_DEST (set))))
    return true;

  set_mode = GET_MODE (SET_DEST (set));
  if (!ccmodes_compatible(set_mode, req_mode))
    return false;
  return (GET_MODE (SET_SRC (set)) == set_mode);
}

/* Return true if every SET in INSN that sets the CC register
   has source and destination with matching CC modes and that
   CC mode is at least as constrained as REQ_MODE.
   If REQ_MODE is VOIDmode, always return false.  */

bool
propeller_match_ccmode (rtx insn, enum machine_mode req_mode)
{
  int i;

  if (req_mode == VOIDmode)
    return false;

  if (GET_CODE (PATTERN (insn)) == SET)
    return propeller_match_ccmode_set (PATTERN(insn), req_mode);

  if (GET_CODE (PATTERN (insn)) == PARALLEL)
      for (i = 0; i < XVECLEN (PATTERN (insn), 0); i++)
        {
          rtx set = XVECEXP (PATTERN (insn), 0, i);
          if (GET_CODE (set) == SET)
            if (!propeller_match_ccmode_set (set, req_mode))
              return false;
        }

  return true;
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
	offset = current_frame_info.callee_size - UNITS_PER_WORD;
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
  for (regno = 0; regno <= PROP_FP_REGNUM; regno++)
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
  if (df_regs_ever_live_p (PROP_LR_REGNUM))
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
  if (is_native_function (current_function_decl))
  {
    rtx current_func_sym = XEXP (DECL_RTL (current_function_decl), 0);
    emit_jump_insn (gen_native_return (current_func_sym));
  }
  else
  {
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

  if (is_native_function (current_function_decl))
    return 0;

  if (df_regs_ever_live_p (PROP_LR_REGNUM) || crtl->profile)
    return 0;

  if (propeller_compute_frame_size (get_frame_size ()) != 0)
    return 0;

  return 1;
}

/*
 * expand a call to the appropriate instructions, depending on whether
 * we need a native or regular call
 * returns true if expansion is finished, false if the normal pattern
 * matching needs to be done
 */
bool
propeller_expand_call (rtx setreg, rtx dest, rtx numargs)
{
    rtx callee = XEXP (dest, 0);
    if (GET_CODE (callee) == SYMBOL_REF)
    {
        if (is_native_function (SYMBOL_REF_DECL (callee)))
        {
            rtx pat;

            if (callee == XEXP (DECL_RTL (current_function_decl), 0))
            {
                error("native function cannot be recursive");
            }
            if (setreg == NULL_RTX) {
                pat = gen_call_native (callee, numargs);
            } else {
                pat = gen_call_native_value (setreg, callee, numargs);
            }
            emit_call_insn (pat);
            return true;
        }
    }
    return false;
}

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
        return TARGET_LMM;
    case CONST_INT:
        return TARGET_LMM || (INTVAL (x) >= -511 && INTVAL (x) <= 511);
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
    case 'M': return value >= -512 && value < 0;
    case 'N': return value >= -511 && value <= 0;
    case 'W': return value < 0 && value > 511;
    default:
        gcc_unreachable();
    }
}

/*
 * code for selecting which section a constant or declaration should go in
 */

/* where should we put a constant?
 * in Cog mode, only integer constants should go in text,
 * everything else should go in data
 * in XMM mode things are even more severe, no constants
 * at all should go in text
 */
static section *
propeller_select_rtx_section (enum machine_mode mode, rtx x,
			      unsigned HOST_WIDE_INT align)
{
  if (!TARGET_LMM)
    {
      if (GET_MODE_SIZE (mode) >= BITS_PER_UNIT
	  || mode == BLKmode)
	{
	  return data_section;
	}
      return default_elf_select_rtx_section (mode, x, align);
    }
  /* in XMM mode we cannot access constants in the data section
     except for that, XMM and LMM are pretty compatible, so let's
     just always put constants in data */
  return data_section;
}

static section *
propeller_select_section (tree decl, int reloc, unsigned HOST_WIDE_INT align)
{
  /* put constants into the data section (in hub ram) */
  if (TREE_CODE (decl) != VAR_DECL && TREE_CODE (decl) != FUNCTION_DECL)
    return data_section;

  return default_elf_select_section (decl, reloc, align);
}


/*
 * builtin functions
 * here are the builtins we support:
 *
 * unsigned __builtin_cogid(void)
 *     get the current cog id
 * unsigned __builtin_coginit(unsigned mode)
 *     start or restart a cog
 * void __builtin_cogstop(unsigned id)
 *     stop a cog
 *
 * unsigned __builtin_reverse(unsigned x, unsigned n)
 *     reverse the bottom n bits of x, and 0 the others
 * unsigned __builtin_waitcnt(unsigned c, unsigned d)
 *     wait until the frequency counter reaches c;
 *     returns c+d
 * void __builtin_waitpeq(unsigned state, unsigned mask)
 *     wait until (INA & mask) == state
 * void __builtin_waitpne(unsigned state, unsigned mask)
 *     wait until (INA & mask) != state
 * void __builtin_waitvid(unsigned colors, unsigned pixels)
 *     wait for video generator
 *
 * void * __builtin_taskswitch(void *newfunc)
 *     switch to a new function
 *
 */

enum propeller_builtins
{
    PROPELLER_BUILTIN_COGID,
    PROPELLER_BUILTIN_COGINIT,
    PROPELLER_BUILTIN_COGSTOP,
    PROPELLER_BUILTIN_REVERSE,

    PROPELLER_BUILTIN_WAITCNT,
    PROPELLER_BUILTIN_WAITPEQ,
    PROPELLER_BUILTIN_WAITPNE,
    PROPELLER_BUILTIN_WAITVID,

    PROPELLER_BUILTIN_TASKSWITCH
};

/* Initialise the builtin functions.  Start by initialising
   descriptions of different types of functions (e.g., void fn(int),
   int fn(void)), and then use these to define the builtins.
*/
static void
propeller_init_builtins (void)
{
  tree endlink = void_list_node;
  tree uns_endlink = tree_cons (NULL_TREE, unsigned_type_node, endlink);
  tree uns_uns_endlink = tree_cons (NULL_TREE, unsigned_type_node, uns_endlink);
  tree ptr_endlink = tree_cons (NULL_TREE, ptr_type_node, endlink);

  tree void_ftype_void, void_ftype_uns, void_ftype_uns_uns;
  tree vfunc_ftype_vfunc;
  tree uns_ftype_void;
  tree uns_ftype_uns;
  tree uns_ftype_uns_uns;

  /* void func (void) */
  void_ftype_void = build_function_type (void_type_node, endlink);

  /* void func (unsigned) */
  void_ftype_uns = build_function_type (void_type_node, uns_endlink);
  void_ftype_uns_uns = build_function_type (void_type_node, uns_uns_endlink);

  /* unsigned func(void) */
  uns_ftype_void = build_function_type (unsigned_type_node, endlink);

  /* unsigned func(unsigned) */
  /* unsigned func(unsigned,unsigned) */
  uns_ftype_uns = build_function_type (unsigned_type_node, uns_endlink);
  uns_ftype_uns_uns = build_function_type (unsigned_type_node, uns_uns_endlink);

  /* void (*)(void) func(void (*f)(void)) */
  vfunc_ftype_vfunc = build_function_type(ptr_type_node, ptr_endlink);

  add_builtin_function("__builtin_cogid", uns_ftype_void,
                       PROPELLER_BUILTIN_COGID,
                       BUILT_IN_MD, NULL, NULL_TREE);
  add_builtin_function("__builtin_coginit", uns_ftype_uns,
                       PROPELLER_BUILTIN_COGINIT,
                       BUILT_IN_MD, NULL, NULL_TREE);
  add_builtin_function("__builtin_cogstop", void_ftype_uns,
                       PROPELLER_BUILTIN_COGSTOP,
                       BUILT_IN_MD, NULL, NULL_TREE);
  add_builtin_function("__builtin_reverse", uns_ftype_uns_uns,
                       PROPELLER_BUILTIN_REVERSE,
                       BUILT_IN_MD, NULL, NULL_TREE);
  add_builtin_function("__builtin_waitcnt", uns_ftype_uns_uns,
                       PROPELLER_BUILTIN_WAITCNT,
                       BUILT_IN_MD, NULL, NULL_TREE);
  add_builtin_function("__builtin_waitpeq", void_ftype_uns_uns,
                       PROPELLER_BUILTIN_WAITPEQ,
                       BUILT_IN_MD, NULL, NULL_TREE);
  add_builtin_function("__builtin_waitpne", void_ftype_uns_uns,
                       PROPELLER_BUILTIN_WAITPNE,
                       BUILT_IN_MD, NULL, NULL_TREE);
  add_builtin_function("__builtin_waitvid", void_ftype_uns_uns,
                       PROPELLER_BUILTIN_WAITVID,
                       BUILT_IN_MD, NULL, NULL_TREE);

  add_builtin_function("__builtin_taskswitch", vfunc_ftype_vfunc,
                       PROPELLER_BUILTIN_TASKSWITCH,
                       BUILT_IN_MD, NULL, NULL_TREE);

}

/* Given a builtin function taking 2 operands (i.e., target + source),
   emit the RTL for the underlying instruction. */
static rtx
propeller_expand_builtin_2op (enum insn_code icode, tree call, rtx target)
{
  tree arg0;
  rtx op0, pat;
  enum machine_mode tmode, mode0;

  /* Grab the incoming argument and emit its RTL. */
  arg0 = CALL_EXPR_ARG (call, 0);
  op0 = expand_expr (arg0, NULL_RTX, VOIDmode, EXPAND_NORMAL);

  /* Determine the modes of the instruction operands. */
  tmode = insn_data[icode].operand[0].mode;
  mode0 = insn_data[icode].operand[1].mode;

  /* Ensure that the incoming argument RTL is in a register of the
     correct mode. */
  if (!(*insn_data[icode].operand[1].predicate) (op0, mode0))
      op0 = copy_to_mode_reg (mode0, op0);
  

  /* If there isn't a suitable target, emit a target register. */
  if (target == 0
      || GET_MODE (target) != tmode
      || !(*insn_data[icode].operand[0].predicate) (target, tmode))
  {
    target = gen_reg_rtx (tmode);
  }
  /* Emit and return the new instruction. */
  pat = GEN_FCN (icode) (target, op0);
  if (!pat)
    return 0;
  emit_insn (pat);

  return target;

}
/* Expand a builtin function which takes two arguments, and returns a void. */
static rtx
propeller_expand_builtin_2opvoid (enum insn_code icode, tree call)
{
  tree arg0, arg1;
  rtx op0, op1, pat;
  enum machine_mode mode0, mode1;

  /* Grab the function's arguments. */
  arg0 = CALL_EXPR_ARG (call, 0);
  arg1 = CALL_EXPR_ARG (call, 1);

  /* Emit rtl sequences for the function arguments. */
  op0 = expand_expr (arg0, NULL_RTX, VOIDmode, EXPAND_NORMAL);
  op1 = expand_expr (arg1, NULL_RTX, VOIDmode, EXPAND_NORMAL);

  /* Get the mode's of each of the instruction operands. */
  mode0 = insn_data[icode].operand[0].mode;
  mode1 = insn_data[icode].operand[1].mode;

  /* Ensure that each of the function argument rtl sequences are in a
     register of the correct mode. */
  if (!(*insn_data[icode].operand[0].predicate) (op0, mode0))
    op0 = copy_to_mode_reg (mode0, op0);
  if (!(*insn_data[icode].operand[1].predicate) (op1, mode1))
    op1 = copy_to_mode_reg (mode1, op1);

  /* Emit and return the new instruction. */
  pat = GEN_FCN (icode) (op0, op1);
  if (!pat)
    return 0;
  emit_insn (pat);

  return NULL_RTX;

}
static rtx
propeller_expand_builtin_1opvoid (enum insn_code icode, tree call)
{
  tree arg0;
  rtx op0, pat;
  enum machine_mode mode0;

  /* Grab the function's arguments. */
  arg0 = CALL_EXPR_ARG (call, 0);

  /* Emit rtl sequences for the function arguments. */
  op0 = expand_expr (arg0, NULL_RTX, VOIDmode, EXPAND_NORMAL);

  /* Get the mode's of each of the instruction operands. */
  mode0 = insn_data[icode].operand[0].mode;

  /* Ensure that each of the function argument rtl sequences are in a
     register of the correct mode. */
  if (!(*insn_data[icode].operand[0].predicate) (op0, mode0))
    op0 = copy_to_mode_reg (mode0, op0);

  /* Emit and return the new instruction. */
  pat = GEN_FCN (icode) (op0);
  if (!pat)
    return 0;
  emit_insn (pat);

  return NULL_RTX;

}
/* Given a builtin function taking 3 operands (i.e., target + two
   source), emit the RTL for the underlying instruction. */
static rtx
propeller_expand_builtin_3op (enum insn_code icode, tree call, rtx target)
{
  tree arg0, arg1;
  rtx op0, op1, pat;
  enum machine_mode tmode, mode0, mode1;

  /* Grab the function's arguments. */
  arg0 = CALL_EXPR_ARG (call, 0);
  arg1 = CALL_EXPR_ARG (call, 1);

  /* Emit rtl sequences for the function arguments. */
  op0 = expand_expr (arg0, NULL_RTX, VOIDmode, EXPAND_NORMAL);
  op1 = expand_expr (arg1, NULL_RTX, VOIDmode, EXPAND_NORMAL);

  /* Get the mode's of each of the instruction operands. */
  tmode = insn_data[icode].operand[0].mode;
  mode0 = insn_data[icode].operand[1].mode;
  mode1 = insn_data[icode].operand[2].mode;

  /* Ensure that each of the function argument rtl sequences are in a
     register of the correct mode. */
  if (!(*insn_data[icode].operand[1].predicate) (op0, mode0))
    op0 = copy_to_mode_reg (mode0, op0);
  if (!(*insn_data[icode].operand[2].predicate) (op1, mode1))
    op1 = copy_to_mode_reg (mode1, op1);

  /* If no target has been given, create a register to use as the target. */
  if (target == 0
      || GET_MODE (target) != tmode
      || !(*insn_data[icode].operand[0].predicate) (target, tmode))
    target = gen_reg_rtx (tmode);

  /* Emit and return the new instruction. */
  pat = GEN_FCN (icode) (target, op0, op1);
  if (!pat)
    return 0;
  emit_insn (pat);

  return target;

}

static rtx
propeller_expand_builtin_1op(enum insn_code icode, rtx target)
{
  enum machine_mode tmode;
  rtx pat;

  tmode = insn_data[icode].operand[0].mode;
  /* If no target has been given, create a register to use as the target. */
  if (target == 0
      || GET_MODE (target) != tmode
      || !(*insn_data[icode].operand[0].predicate) (target, tmode))
    target = gen_reg_rtx (tmode);

  pat = GEN_FCN (icode) (target);
  if (!pat)
    return 0;
  emit_insn (pat);
  return target;
}

/* Expand a call to a builtin function */
static rtx
propeller_expand_builtin (tree exp, rtx target, rtx subtarget ATTRIBUTE_UNUSED,
			 enum machine_mode mode ATTRIBUTE_UNUSED,
			 int ignore ATTRIBUTE_UNUSED)
{
  tree fndecl = TREE_OPERAND (CALL_EXPR_FN (exp), 0);
  int fcode = DECL_FUNCTION_CODE (fndecl);

  switch (fcode)
    {
    case PROPELLER_BUILTIN_COGID:
        return propeller_expand_builtin_1op (CODE_FOR_cogid, target);
    case PROPELLER_BUILTIN_COGINIT:
        return propeller_expand_builtin_2op (CODE_FOR_coginit, exp, target);
    case PROPELLER_BUILTIN_COGSTOP:
        return propeller_expand_builtin_1opvoid (CODE_FOR_cogstop, exp);
    case PROPELLER_BUILTIN_REVERSE:
        return propeller_expand_builtin_3op (CODE_FOR_reverse, exp, target);
    case PROPELLER_BUILTIN_WAITCNT:
        return propeller_expand_builtin_3op (CODE_FOR_waitcnt, exp, target);
    case PROPELLER_BUILTIN_WAITPEQ:
        return propeller_expand_builtin_2opvoid (CODE_FOR_waitpeq, exp);
    case PROPELLER_BUILTIN_WAITPNE:
        return propeller_expand_builtin_2opvoid (CODE_FOR_waitpne, exp);
    case PROPELLER_BUILTIN_TASKSWITCH:
        return propeller_expand_builtin_2op (CODE_FOR_taskswitch, exp, target);
    default:
        gcc_unreachable ();
    }
  return NULL_RTX;
}

/* checks for forward branches */
bool
propeller_forward_branch_p (rtx insn)
{
  rtx lab = JUMP_LABEL (insn);

  /* The INSN must have a jump label.  */
  gcc_assert (lab != NULL_RTX);

  if (INSN_ADDRESSES_SET_P ())
    return INSN_ADDRESSES (INSN_UID (lab)) > INSN_ADDRESSES (INSN_UID (insn));  

  while (insn)
    {
      if (insn == lab)
	return true;
      else
	insn = NEXT_INSN (insn);
    }

  return false;
}

/*
 * helper function
 * extract the destination of a call insn
 */
static rtx
get_call_dest(rtx branch)
{
  rtx call;
  call = PATTERN (branch);
  /* there might be a parallel in here with a clobber */
  if (GET_CODE (call) == PARALLEL)
    call = XVECEXP (call, 0, 0);

  if (GET_CODE (call) == SET)
    {
      call = SET_SRC (call);
    }
  if (GET_CODE (call) != CALL)
    gcc_unreachable ();
  return XEXP (XEXP (call, 0), 0);
}

/*
 * helper function
 * extract the destination of a jump insn
 * returns the RETURN insn for a return
 */
static rtx
get_jump_dest(rtx branch)
{
  rtx call;
  rtx set, src;

  call = PATTERN (branch);

  /*
   * this may be a parallel (e.g. a (return) and use)
   * all of our parallel branches are set up so that
   * the branch is first
   */
  if (GET_CODE (call) == PARALLEL)
    {
      call = XVECEXP (call, 0, 0);
    }
  if (GET_CODE (call) == RETURN)
    {
      return call;
    }

  if (GET_CODE (call) == ADDR_VEC
      || GET_CODE (call) == ADDR_DIFF_VEC)
    return NULL;

  /* watch out for user asm */
  if (extract_asm_operands (call) != NULL)
    return NULL;

  set = single_set (branch);
  src = SET_SRC (set);

  if (GET_CODE (SET_DEST (set)) != PC)
    abort ();
  if (GET_CODE (src) == IF_THEN_ELSE)
    {
      if (GET_CODE (XEXP (src, 1)) != PC)
	src = XEXP (src, 1);
      else
	src = XEXP (src, 2);
    }

  return src;
}

/*
 * dest_ok_for_fcache: returns true if a jump destination is OK
 * for fcache mode
 */
static bool
dest_ok_for_fcache(rtx dest)
{
  if (!dest)
    return false;
  if (GET_CODE (dest) == RETURN)
    return true;
  if (GET_CODE (dest) != LABEL_REF)
    return false;

  return true;
}

/*
 * see if we can load this function into internal memory (fcache)
 * the final requirements are in place:
 * (1) the function must fit (be < MAX_FCACHE_SIZE bytes long)
 * (2) it can be recursive, but no other function calls may be made
 * sets the "recursive" flag depending on whether or not the function
 * is recursive
 */
#define MAX_FCACHE_SIZE (512)

static bool
fcache_func_ok (bool *recursive)
{
  rtx dest;
  rtx anchor = get_insns ();
  rtx insn;
  HOST_WIDE_INT total_len = 0;
  int loop_count = 0;

  if (dump_file)
    fprintf(dump_file, "considering %s for fcache\n", current_function_name ());

  *recursive = false;
  for (insn = anchor; insn; insn = NEXT_INSN (insn))
    {
      if (!INSN_P (insn))
	continue;

      if (GET_CODE (insn) == CALL_INSN)
	{
	  /* check for recursive call */
	  dest = get_call_dest (insn);
	  if (!dest || GET_CODE(dest) != SYMBOL_REF)
	    {
	      if (dump_file)
		{
		  fprintf (dump_file, "call not to a symbol\n");
		}
	      return false;
	    }
	  /* allow recursive functions */
	  if (dest == XEXP (DECL_RTL (current_function_decl), 0))
	    {
	      loop_count++;
	      *recursive = true;
	    }
	  else
	    {	  
	      if (dump_file)
		{
		  fprintf (dump_file, "not a leaf function\n");
		}
	      return false;
	    }
	}
      else if (GET_CODE (insn) == JUMP_INSN)
	{
	  dest = get_jump_dest (insn);
	  if (!dest_ok_for_fcache (dest))
	    {
	      if (dump_file)
		{
		  fprintf (dump_file, "jump not to a label\n");
		}
	      return false;
	    }
	  if (GET_CODE (dest) == LABEL_REF
	      && !propeller_forward_branch_p (insn))
	    {
	      loop_count++;
	    }
	}
      total_len += get_attr_length (insn);
      if (total_len > MAX_FCACHE_SIZE)
	{
	  if (dump_file)
	    {
	      fprintf (dump_file, "function too long to fit in fcache\n");
	    }
	  return false;
	}
    } 

  if (loop_count == 0)
    {
      if (dump_file)
	fprintf (dump_file, "no loops in function\n");
      return false;
    }

  return true;
}

/*
 * convert a CALL insn to fcache mode
 */
static void
fcache_convert_call (rtx insn)
{
  rtx pattern;
  rtx addr;

  pattern = PATTERN (insn);
  /* look inside parallels if necessary (assume branches are at start) */
  if (GET_CODE (pattern) == PARALLEL)
    pattern = XVECEXP (pattern, 0, 0);

  /* handle call_value */
  if (GET_CODE (pattern) == SET)
    pattern = SET_SRC(pattern);

  if (GET_CODE (pattern) == CALL)
    {
      if (dump_file) fprintf (dump_file, "replacing call\n");
      if (GET_CODE (pattern) == SET)
	pattern = SET_SRC (pattern);
      if (GET_CODE (pattern) != CALL)
	gcc_unreachable ();
      pattern = XEXP (pattern, 0);
      if (GET_CODE (pattern) != MEM)
	gcc_unreachable ();
      addr = gen_rtx_UNSPEC ( Pmode, gen_rtvec (1, XEXP (pattern, 0)),
			      UNSPEC_FCACHE_CALL );
      XEXP (pattern, 0) = addr;
    }
  else
    {
      gcc_unreachable ();
    }
}

/*
 * convert a jump insn
 * this pretty much requires that we re-generate the insn
 * "fcache_base" is the label at the start of the fcache block
 */
static void
fcache_convert_jump(rtx orig_insn, rtx fcache_base)
{
  rtx pattern;
  rtx src;
  rtx next;
  rtx addr;

  pattern = PATTERN (orig_insn);

  /*
   * delete the original insn
   */
  next = delete_insn (orig_insn);

  /*
   * this may be a parallel (e.g. a (return) and use)
   * all of our parallel branches are set up so that
   * the branch is first
   */
  if (GET_CODE (pattern) == PARALLEL)
    {
      src = XVECEXP (pattern, 0, 0);
      if (GET_CODE (src) == RETURN)
	{
	  addr = gen_rtx_UNSPEC (Pmode, gen_rtvec (1, src),
				 UNSPEC_FCACHE_RET);
	  XVECEXP (pattern, 0, 0) = addr;
	}
    }
  else if (GET_CODE (pattern) == RETURN)
    {
      pattern = gen_rtx_UNSPEC (Pmode, gen_rtvec (1, pattern),
				UNSPEC_FCACHE_RET);
    }
  else if (GET_CODE (pattern) == ADDR_VEC
	   || GET_CODE (pattern) == ADDR_DIFF_VEC
	   || extract_asm_operands (pattern) != NULL)
    {
      gcc_unreachable ();
    }
  else
    {
      src = SET_SRC (pattern);

      if (GET_CODE (SET_DEST (pattern)) != PC)
	abort ();
      if (GET_CODE (src) == IF_THEN_ELSE)
	{
	  if (GET_CODE (XEXP (src, 1)) != PC)
	    {
	      addr = XEXP (src, 1);
	      XEXP (src, 1) = gen_rtx_UNSPEC (Pmode, gen_rtvec (2, addr, fcache_base), UNSPEC_FCACHE_LABEL_REF);
	    }
	  else
	    {
	      addr = XEXP (src, 2);
	      XEXP (src, 2) = gen_rtx_UNSPEC (Pmode, gen_rtvec (2, addr, fcache_base), UNSPEC_FCACHE_LABEL_REF);
	    }
	}
      else
	{
	  SET_SRC (pattern) = gen_rtx_UNSPEC (Pmode, gen_rtvec (2, src, fcache_base), UNSPEC_FCACHE_LABEL_REF);
	}
    }

  /* generate a new insn with the given pattern */
  emit_jump_insn_before (pattern, next);
}

/*
 * convert a function so it will work in fcache space
 * this basically means converting all label references to
 * (unspec FCACHE_LABEL_REF), returns to (unspec FCACHE_RET),
 * and fixing up immediate moves
 */
static void
fcache_func_reorg (bool recursive)
{
  rtx start_label, end_label;
  rtx anchor, last;
  rtx insn;
  rtx pattern;

  if (dump_file)
    fprintf(dump_file, " *** trying to put %s into fcache\n", current_function_name ());

  /* first, we have to insert FCACHE_LOAD and the start and end labels */
  /* find first and last real insns */
  anchor = get_insns ();
  while (!INSN_P (anchor))
    anchor = NEXT_INSN (anchor);
  last = NULL;
  insn = NEXT_INSN (anchor);
  while (insn)
    {
      if (INSN_P (insn))
	last = insn;
      insn = NEXT_INSN (insn);
    }
  start_label = gen_label_rtx ();
  LABEL_NUSES (start_label)++;
  LABEL_PRESERVE_P (start_label) = 1;
  anchor = emit_label_before (start_label, anchor);
  INSN_ADDRESSES_NEW (anchor, -1);
  start_label = gen_rtx_LABEL_REF (VOIDmode, start_label);

  end_label = gen_label_rtx ();
  LABEL_NUSES (end_label)++;
  LABEL_PRESERVE_P (end_label) = 1;
  insn = emit_label_after (end_label, last);
  INSN_ADDRESSES_NEW (insn, -1);

  end_label = gen_rtx_LABEL_REF (VOIDmode, end_label);

  /* now add in the FCACHE_LOAD */
  insn = gen_rtx_UNSPEC_VOLATILE (VOIDmode, gen_rtvec (2, start_label, end_label),
			 UNSPEC_FCACHE_LOAD);

  anchor = emit_insn_before (insn, anchor);
  INSN_ADDRESSES_NEW (anchor, -1);

  /* emit code to get the return correct */
  /* we have to output
       mov pc,lr
       mov lr,__LMM_RET
     at the start of the function
     we do this with a special pattern named "fcache_func_start"
  */
  insn = gen_fcache_func_start ();
  anchor = emit_insn_after (insn, anchor);
  INSN_ADDRESSES_NEW (anchor, -1);

  /* finally adjust all instructions for fcache mode */
  for (insn = anchor; insn; insn = NEXT_INSN (insn))
    {
      if (!INSN_P (insn)) continue;

      if (dump_file)
	{
	  fprintf (dump_file, "considering: ");
	  print_rtl_single (dump_file, insn);
	}
      if (GET_CODE (insn) == CALL_INSN)
	{
	  fcache_convert_call (insn);
	}
      else if (GET_CODE (insn) == JUMP_INSN)
	{
	  fcache_convert_jump (insn, start_label);
	}
      else
	{
	  pattern = PATTERN (insn);
	  if ( GET_CODE (pattern) == SET
	       && propeller_big_const (SET_SRC (pattern), SImode) )
	    
	    {
	      gcc_unreachable ();
	    }
	}
    }
}

/*
 * a machine dependent pass over the rtl
 * this is a chance for us to do additional machine specific
 * optimizations
 */

static void
propeller_reorg(void)
{
  FILE *save_dump_file;
  bool fcache_recursive;

  /* for now, this does nothing */
  if (!TARGET_EXPERIMENTAL)
    return;

  save_dump_file = dump_file;
  dump_file = stderr;

  if (TARGET_LMM)
    {
      if (fcache_func_ok (&fcache_recursive))
	{
	  fcache_func_reorg (fcache_recursive);
	}
    }
  dump_file = save_dump_file;
}


#undef TARGET_DEFAULT_TARGET_FLAGS
#define TARGET_DEFAULT_TARGET_FLAGS (TARGET_DEFAULT)
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

#undef TARGET_EXCEPT_UNWIND_INFO
#define TARGET_EXCEPT_UNWIND_INFO sjlj_except_unwind_info

#undef TARGET_INIT_BUILTINS
#define TARGET_INIT_BUILTINS propeller_init_builtins
#undef TARGET_EXPAND_BUILTIN
#define TARGET_EXPAND_BUILTIN propeller_expand_builtin

#undef TARGET_MACHINE_DEPENDENT_REORG
#define TARGET_MACHINE_DEPENDENT_REORG propeller_reorg

#undef TARGET_ASM_SELECT_SECTION
#define TARGET_ASM_SELECT_SECTION propeller_select_section
#undef TARGET_ASM_SELECT_RTX_SECTION
#define TARGET_ASM_SELECT_RTX_SECTION propeller_select_rtx_section

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
