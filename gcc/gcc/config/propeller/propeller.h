/* Definitions of target machine for GNU compiler, Propeller architecture.
   Contributed by Eric Smith <ersmith@totalspectrum.ca>

   Copyright (C) 2011 Parallax, Inc.

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

#ifndef GCC_PROPELLER_H
#define GCC_PROPELLER_H

/*-------------------------------*/
/* Config for gas and binutils   */
/*-------------------------------*/
#undef  STARTFILE_SPEC
#define STARTFILE_SPEC "\
%{mxmm*:hubstart_xmm.o%s; mp2:p2vectors.o%s; :spinboot.o%s} \
%{mcog:%{mp2:crt0_cog2.o%s; :crt0_cog.o%s}; :%{g:_crt0_debug.o%s; :_crt0.o%s} _crtbegin.o%s} \
"

#undef  ENDFILE_SPEC
#define ENDFILE_SPEC "%{mcog:%{mp2:crtend_cog2.o%s; :crtend_cog.o%s}; :_crtend.o%s}"

#undef ASM_SPEC
#define ASM_SPEC "\
%{!mpasm: \
  %{!mcog:-lmm} \
  %{mcmm:-cmm}  \
  %{mp2:-p2} \
  %{mrelax:-relax}} \
"
#undef LIB_SPEC
#define LIB_SPEC "                              \
%{mcog: %{mp2:-lcog2; :-lcog};                  \
  :  --start-group -lc -lgcc --end-group        \
  }                                             \
"

#undef LINK_SPEC
#define LINK_SPEC "                                             \
%{mrelax:-relax}                                                \
%{mp2: %{mcog:-mpropeller_cog; mxmmc:-mpropeller2_xmmc; mxmm:-mpropeller2_xmm; :-mpropeller2};                   \
     : %{mcog:-mpropeller_cog; mxmmc:-mpropeller_xmmc; mxmm:-mpropeller_xmm; mxmm-single:-mpropeller_xmm_single; mxmm-split:-mpropeller_xmm_split; :-mpropeller}} \
"

#define TARGET_DEFAULT (MASK_LMM | MASK_64BIT_DOUBLES)

/*-------------------------------*/
/* Run-time Target Specification */
/*-------------------------------*/

/* Print subsidiary information on the compiler version in use.  */
#ifndef TARGET_VERSION
#define TARGET_VERSION fprintf (stderr, " (Propeller)")
#endif

/* Target CPU builtins.  */
#define TARGET_CPU_CPP_BUILTINS()                            \
  do                                                         \
    {                                                        \
      builtin_define ("__propeller__");                      \
      builtin_define ("__PROPELLER__");                      \
      builtin_assert ("cpu=propeller");                      \
      builtin_assert ("machine=propeller");                  \
      if (TARGET_XMM)                                        \
        {                                                    \
          builtin_define ("__PROPELLER_XMM__");              \
          builtin_define ("__PROPELLER_USE_XMM__");          \
        }                                                    \
      else if (TARGET_XMM_CODE)                              \
        {                                                    \
          builtin_define ("__PROPELLER_XMMC__");             \
          builtin_define ("__PROPELLER_USE_XMM__");          \
        }                                                    \
      else if (TARGET_CMM)                                   \
        builtin_define ("__PROPELLER_CMM__");                \
      else if (TARGET_LMM)                                   \
        builtin_define ("__PROPELLER_LMM__");                \
      else                                                   \
        builtin_define ("__PROPELLER_COG__");                \
      if (TARGET_P2)                                         \
        builtin_define ("__PROPELLER2__");                   \
      if (TARGET_64BIT_DOUBLES)                              \
        builtin_define ("__PROPELLER_64BIT_DOUBLES__");      \
      else                                                   \
        builtin_define ("__PROPELLER_32BIT_DOUBLES__");      \
    }                                                        \
  while (0)

/* we always have C99 functions available if floating point is used */
#undef TARGET_C99_FUNCTIONS
#define TARGET_C99_FUNCTIONS 1

/*---------------------------------*/
/* Target machine storage layout.  */
/*---------------------------------*/

#define BITS_BIG_ENDIAN 0
#define BYTES_BIG_ENDIAN 0
#define WORDS_BIG_ENDIAN 0

#define BITS_PER_UNIT 8
#define BITS_PER_WORD 32
#define UNITS_PER_WORD 4

#define POINTER_SIZE 32

#define PROMOTE_MODE(MODE,UNSIGNEDP,TYPE)               \
do {                                                    \
  if (GET_MODE_CLASS (MODE) == MODE_INT                 \
      && GET_MODE_SIZE (MODE) < UNITS_PER_WORD)         \
    (MODE) = word_mode;                                 \
} while (0)

#define PARM_BOUNDARY 32

#define STACK_BOUNDARY 32

#define BIGGEST_ALIGNMENT 32

#define FUNCTION_BOUNDARY  ((TARGET_CMM && !TARGET_P2) ? 8 : 32)

#define EMPTY_FIELD_BOUNDARY 32

#define STRICT_ALIGNMENT 1

#define TARGET_FLOAT_FORMAT IEEE_FLOAT_FORMAT

/* Make strings word-aligned so strcpy from constants will be faster.  */
#define CONSTANT_ALIGNMENT(EXP, ALIGN)  \
  (TREE_CODE (EXP) == STRING_CST        \
   && (ALIGN) < BITS_PER_WORD ? BITS_PER_WORD : (ALIGN))

/* Make arrays and structures word-aligned to allow faster copying etc.  */
#define DATA_ALIGNMENT(TYPE, ALIGN)                                     \
  ((((ALIGN) < BITS_PER_WORD)                                           \
    && (TREE_CODE (TYPE) == ARRAY_TYPE                                  \
        || TREE_CODE (TYPE) == UNION_TYPE                               \
        || TREE_CODE (TYPE) == RECORD_TYPE)) ? BITS_PER_WORD : (ALIGN))

/* We need this for the same reason as DATA_ALIGNMENT, namely to cause
   character arrays to be word-aligned so that `strcpy' calls that copy
   constants to character arrays can be done inline, and 'strcmp' can be
   optimised to use word loads.  */
#define LOCAL_ALIGNMENT(TYPE, ALIGN) \
  DATA_ALIGNMENT (TYPE, ALIGN)

/* Nonzero if access to memory by bytes is slow and undesirable.  */
#define SLOW_BYTE_ACCESS 0

#define SLOW_UNALIGNED_ACCESS(MODE, ALIGN) 1

/* Define if operations between registers always perform the operation
   on the full register even if a narrower mode is specified.  */
#define WORD_REGISTER_OPERATIONS

/* Try to generate sequences that don't involve branches, we can then use
   conditional instructions */
#define BRANCH_COST(speed_p, predictable_p) \
  (TARGET_LMM ? 2 : 4)

/*-------------*/
/* Profiling.  */
/*-------------*/

#define FUNCTION_PROFILER(FILE, LABELNO)

/*---------------*/
/* Trampolines.  */
/*---------------*/

#define TRAMPOLINE_SIZE         16

/*----------------------------------------*/
/* Layout of source language data types.  */
/*----------------------------------------*/
#define INT_TYPE_SIZE               32
#define SHORT_TYPE_SIZE             16
#define LONG_TYPE_SIZE              32
#define LONG_LONG_TYPE_SIZE         64

/* size of C floating point types */
#define FLOAT_TYPE_SIZE             32
#define DOUBLE_TYPE_SIZE            (TARGET_64BIT_DOUBLES ? 64 : 32)
#define LONG_DOUBLE_TYPE_SIZE       64

#define DEFAULT_SIGNED_CHAR         0

#define SIZE_TYPE "unsigned int"

#define PTRDIFF_TYPE "int"

/* An alias for the machine mode for pointers.  */
#define Pmode         SImode

/* An alias for the machine mode used for memory references to
   functions being called, in `call' RTL expressions.  */
#define FUNCTION_MODE Pmode

/* Specify the machine mode that this machine uses
   for the index in the tablejump instruction.  */
#define CASE_VECTOR_MODE Pmode


/*---------------------------*/
/* Standard register usage.  */
/*---------------------------*/

/* The propeller is an unusual machine in that it has 512 registers;
 * but the code (at least in non-LMM mode) is actually stored in
 * the register space, taking up some registers. In LMM mode the
 * registers hold a simple interpreter that reads instructions from
 * external RAM and executes them.
 * To simplify things we reserve some registers for traditional uses:
 * 16 general purposes registers (r0-r15), a stack pointer sp, and
 * a program counter pc (which is only used in LMM mode).
 * Our registers are numbered:
 *  0-15:  r0-r15
 *  16:    sp
 *  17:    pc
 *  18:    cc (not really accessible)
 *  19:    fake argument pointer (eliminated after register allocation)
 *  20:    fake frame pointer    (eliminated after register allocation)
 * register usage in the ABI is:
 *  r0-r7: available for parameters and not saved
 *  r8-r14: saved across function calls
 *  r15 is used to save return addresses (link register)
 *  r14 is used as the frame pointer
 */
#define REGISTER_NAMES {        \
  "r0", "r1", "r2", "r3",   \
  "r4", "r5", "r6", "r7",   \
  "r8", "r9", "r10", "r11",   \
  "r12", "r13", "r14", "lr",   \
  "sp", "pc", "cc", "?sap", "?sfp" }

/* some utility defines; the _REG definitions come from propeller.md */
/* r14 the frame register, and r15 the
 * link register
 */
#define PROP_R0        0
#define PROP_R1        1
#define PROP_FP_REGNUM (FRAME_REG)
#define PROP_LR_REGNUM (LINK_REG)
#define PROP_SP_REGNUM (SP_REG)
#define PROP_PC_REGNUM (LMM_PC_REG)
#define PROP_CC_REGNUM (CC_REG)   /* not a real register */
#define PROP_FAKEAP_REGNUM 19  /* similarly for the arg pointer */
#define PROP_FAKEFP_REGNUM 20  /* a fake register for tracking the frame pointer until all offsets are known */

#define FIRST_PSEUDO_REGISTER 21

enum reg_class
{
  NO_REGS,
  R0_REGS,
  R1_REGS,
  GENERAL_REGS,
  SP_REGS,
  BASE_REGS,
  SPECIAL_REGS,
  CC_REGS,
  ALL_REGS,
  LIM_REG_CLASSES
};

#define REG_CLASS_CONTENTS \
{ { 0x00000000 }, /* Empty */                      \
  { 0x00000001 }, /* r0 */                 \
  { 0x00000002 }, /* r1 */                 \
  { 0x0000FFFF }, /* general registers */  \
  { 0x00010000 }, /* sp */        \
  { 0x0001FFFF }, /* base registers */        \
  { 0x00020000 }, /* pc */                         \
  { 0x00040000 }, /* cc */                        \
  { 0x0007FFFF }  /* All registers */              \
}

#define N_REG_CLASSES LIM_REG_CLASSES

#define REG_CLASS_NAMES {\
    "NO_REGS", \
    "R0_REGS", \
    "R1_REGS", \
    "GENERAL_REGS", \
    "SP_REGS", \
    "BASE_REGS", \
    "SPECIAL_REGS", \
    "CC_REGS", \
    "ALL_REGS" }

/* A C expression whose value is a register class containing hard
   register REGNO.  */
extern enum reg_class propeller_reg_class[FIRST_PSEUDO_REGISTER];
#define REGNO_REG_CLASS(R) propeller_reg_class[(R)]

#define FIXED_REGISTERS \
{                       \
  0,0,0,0,0,0,0,0,      \
  0,0,0,0,0,0,0,0,      \
  1,1,1,1,1,        \
}

/* 1 for registers not available across function calls
 * these must include the FIXED_REGISTERS and also any registers
 * that can be used without being saved
 *
 * Note that we do not mark the link register lr as being
 * call-clobbered. It's actually the call instruction itself
 * which clobbers the register, and native calls (for example)
 * do not do so.
 *
 * This approach makes it easier to implement sibcalls; unlike
 * normal calls, sibcalls don't clobber lr.
 */
#define CALL_USED_REGISTERS \
{                       \
  1,1,1,1,1,1,1,1,      \
  0,0,0,0,0,0,0,0,      \
  1,1,1,1,1,        \
}

#define REG_ALLOC_ORDER \
    { \
        7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8, \
        16, 18, 17                                            \
    }

/* we can't really copy to/from the CC */
#define AVOID_CCMODE_COPIES 1

/* The number of argument registers available */
#define NUM_ARG_REGS 6

/* The register number of the stack pointer register, which must also
   be a fixed register according to `FIXED_REGISTERS'.  */
#define STACK_POINTER_REGNUM PROP_SP_REGNUM

/* The register number of the frame pointer register, which is used to
   access automatic variables in the stack frame.  */
#define FRAME_POINTER_REGNUM PROP_FAKEFP_REGNUM

/* The register number of the arg pointer register, which is used to
   access the function's argument list.  */
#define ARG_POINTER_REGNUM PROP_FAKEAP_REGNUM

/* register in which the static chain is passed to a function */
#define STATIC_CHAIN_REGNUM 7

/* Definitions for register eliminations.

   This is an array of structures.  Each structure initializes one pair
   of eliminable registers.  The "from" register number is given first,
   followed by "to".  Eliminations of the same "from" register are listed
   in order of preference.
*/
#define HARD_FRAME_POINTER_REGNUM PROP_FP_REGNUM
#define HARD_FRAME_POINTER_IS_FRAME_POINTER 0
#define HARD_FRAME_POINTER_IS_ARG_POINTER 0

#define ELIMINABLE_REGS                                                 \
{ { ARG_POINTER_REGNUM, STACK_POINTER_REGNUM },              \
  { ARG_POINTER_REGNUM, HARD_FRAME_POINTER_REGNUM },         \
  { FRAME_POINTER_REGNUM, STACK_POINTER_REGNUM },            \
  { FRAME_POINTER_REGNUM, HARD_FRAME_POINTER_REGNUM },       \
}

/* This macro is similar to `INITIAL_FRAME_POINTER_OFFSET'.  It
   specifies the initial difference between the specified pair of
   registers.  This macro must be defined if `ELIMINABLE_REGS' is
   defined.  */
#define INITIAL_ELIMINATION_OFFSET(FROM, TO, OFFSET)                    \
  do {                                                                  \
    (OFFSET) = propeller_initial_elimination_offset ((FROM), (TO));             \
  } while (0)

/* A C expression that is nonzero if REGNO is the number of a hard
   register in which function arguments are sometimes passed.  */
#define FUNCTION_ARG_REGNO_P(r) (r >= 0 && r <= 5)

/* A macro whose definition is the name of the class to which a valid
   base register must belong.  A base register is one used in an
   address which is the register value plus a displacement.  */
#define BASE_REG_CLASS BASE_REGS

#define INDEX_REG_CLASS NO_REGS

#define HARD_REGNO_OK_FOR_BASE_P(NUM) \
    ((unsigned) (NUM) <= PROP_SP_REGNUM)

#define MAX_REGS_PER_ADDRESS 1

/* A C expression which is nonzero if register number NUM is suitable
   for use as a base register in operand addresses.  */
#ifdef REG_OK_STRICT
#define REG_STRICT_P 1
#define REGNO_OK_FOR_BASE_P(NUM)                 \
  (HARD_REGNO_OK_FOR_BASE_P(NUM)                 \
   || HARD_REGNO_OK_FOR_BASE_P(reg_renumber[(NUM)]))
#else
#define REG_STRICT_P 0
#define REGNO_OK_FOR_BASE_P(NUM)                 \
  ((NUM) >= FIRST_PSEUDO_REGISTER || HARD_REGNO_OK_FOR_BASE_P(NUM))
#endif

/* A C expression which is nonzero if register number NUM is suitable
   for use as an index register in operand addresses.  */
#define REGNO_OK_FOR_INDEX_P(NUM) 0

/* A C expression for the number of consecutive hard registers,
   starting at register number REGNO, required to hold a value of mode
   MODE.  */
#define HARD_REGNO_NREGS(REGNO, MODE)                      \
  ((GET_MODE_SIZE (MODE) + UNITS_PER_WORD - 1)             \
   / UNITS_PER_WORD)

/* A C expression that is nonzero if it is permissible to store a
   value of mode MODE in hard register number REGNO (or in several
   registers starting with that one).  All gstore registers are 
   equivalent, so we can set this to 1.  */
#define HARD_REGNO_MODE_OK(REGNO,MODE) propeller_hard_regno_mode_ok ((REGNO), (MODE))

/* A C expression that is nonzero if a value of mode MODE1 is
   accessible in mode MODE2 without copying.  */
#define MODES_TIEABLE_P(MODE1, MODE2) propeller_modes_tieable_p ((MODE1), (MODE2))

/* A C expression for the maximum number of consecutive registers of
   class CLASS needed to hold a value of mode MODE.  */
#define CLASS_MAX_NREGS(CLASS, MODE) \
  ((GET_MODE_SIZE (MODE) + UNITS_PER_WORD - 1) / UNITS_PER_WORD)

/* The maximum number of bytes that a single instruction can move
   quickly between memory and registers or between two memory
   locations.  */
#define MOVE_MAX 4
#define TRULY_NOOP_TRUNCATION(op,ip) 1

/* It's faster to call directly than from registers */
#define NO_FUNCTION_CSE 1

/* All load operations zero extend.  */
#define LOAD_EXTEND_OP(MEM) ZERO_EXTEND

#define CLZ_DEFINED_VALUE_AT_ZERO(MODE, VALUE)  ((VALUE) = 32, 1)
#define CTZ_DEFINED_VALUE_AT_ZERO(MODE, VALUE)  ((VALUE) = 32, 1)

#define LEGITIMATE_CONSTANT_P(X) propeller_legitimate_constant_p (X)

/* GO_IF_LEGITIMATE_ADDRESS recognizes an RTL expression
   that is a valid memory address for an instruction.
   The MODE argument is the machine mode for the MEM expression
   that wants to use this address.  */
#define GO_IF_LEGITIMATE_ADDRESS(MODE, X, WIN) \
  { \
    if (propeller_legitimate_address_p (MODE, X, REG_STRICT_P)) goto WIN; \
  }

/* condition code stuff */
#define SELECT_CC_MODE(OP, X, Y) propeller_select_cc_mode(OP, X, Y)

#define REVERSIBLE_CC_MODE(MODE) 1

#define CANONICALIZE_COMPARISON(CODE, OP0, OP1) \
  (CODE) = propeller_canonicalize_comparison (CODE, &(OP0), &(OP1))

/* indicate that we have some complex conditional move instructions
 * that the converter should check against (not currently used)
 */
#define HAVE_COMPLEX_CMOVES 1


/* Passing Arguments in Registers */

/* A C type for declaring a variable that is used as the first
   argument of `FUNCTION_ARG' and other related values.  */
typedef unsigned int CUMULATIVE_ARGS;

#define FUNCTION_ARG_PADDING(MODE, TYPE) upward
#define BLOCK_REG_PADDING(MODE, TYPE, FIRST) upward

/* If defined, the maximum amount of space required for outgoing arguments
   will be computed and placed into the variable
   `current_function_outgoing_args_size'.  No space will be pushed
   onto the stack for each call; instead, the function prologue should
   increase the stack frame size by this amount.  */
#define ACCUMULATE_OUTGOING_ARGS 1

/* A C statement (sans semicolon) for initializing the variable CUM
   for the state at the beginning of the argument list.  
   For propeller, the first arg is passed in register 0  */
#define INIT_CUMULATIVE_ARGS(CUM,FNTYPE,LIBNAME,FNDECL,N_NAMED_ARGS) \
    (CUM = PROP_R0)

/* How Scalar Function Values Are Returned */

/* STACK AND CALLING */

/* Define this macro if pushing a word onto the stack moves the stack
   pointer to a smaller address.  */
#define STACK_GROWS_DOWNWARD

#define INITIAL_FRAME_POINTER_OFFSET(DEPTH) (DEPTH) = 0

/* Offset from the frame pointer to the first local variable slot to
   be allocated.  */
#define STARTING_FRAME_OFFSET (UNITS_PER_WORD)

/* Offset from the stack pointer register to the first location at which
   outgoing arguments are placed */
#define STACK_POINTER_OFFSET  (0)

/* Define this if the above stack space is to be considered part of the
   space allocated by the caller.  */
#define OUTGOING_REG_PARM_STACK_SPACE(FNTYPE) 1
#define STACK_PARMS_IN_REG_PARM_AREA

/* Offset from the argument pointer register to the first argument's
   address.  On some machines it may depend on the data type of the
   function.  */
#define FIRST_PARM_OFFSET(F) (UNITS_PER_WORD)

/* A C expression whose value is RTL representing the location of the
   incoming return address at the beginning of any function, before
   the prologue.  */
#define INCOMING_RETURN_ADDR_RTX  gen_rtx_REG( SImode, PROP_LR_REGNUM)

/* Define this macro as a C expression that is nonzero for registers used by
   the epilogue or return pattern */
#define EPILOGUE_USES(regno) propeller_epilogue_uses(regno)

/* A C expression whose value is RTL representing the value of the return
   address for the frame COUNT steps up from the current frame.  */

#define RETURN_ADDR_RTX(COUNT, FRAME) \
  propeller_return_addr (COUNT, FRAME)

/* A C expression whose value is an integer giving the offset, in bytes,
from the value of the stack pointer register to the top of the stack
frame at the beginning of any function, before the prologue.  The top of
the frame is defined to be the value of the stack pointer in the
previous frame, just before the call instruction.

You only need to define this macro if you want to support call frame
debugging information like that provided by DWARF 2.  */
#define INCOMING_FRAME_SP_OFFSET 0

/*
 * function results
 */
#define FUNCTION_VALUE(VALTYPE, FUNC)                                   \
   gen_rtx_REG ((INTEGRAL_TYPE_P (VALTYPE)                              \
                 && TYPE_PRECISION (VALTYPE) < BITS_PER_WORD)           \
                    ? word_mode                                         \
                    : TYPE_MODE (VALTYPE),                              \
                    PROP_R0)

#define LIBCALL_VALUE(MODE) gen_rtx_REG (MODE, PROP_R0)

#define FUNCTION_VALUE_REGNO_P(N) ((N) == PROP_R0)

#define DEFAULT_PCC_STRUCT_RETURN 0

/*
 * the overall assembler file framework
 */
#define ASM_COMMENT_START "\'"
#define ASM_APP_ON ""
#define ASM_APP_OFF ""

#define FILE_ASM_OP     ""

#define USER_LABEL_PREFIX "_"

#define DOLLARS_IN_IDENTIFIERS 0
#define NO_DOLLAR_IN_LABEL

/* Switch to the text or data segment.  */
extern const char *propeller_text_asm_op;
extern const char *propeller_data_asm_op;
extern const char *propeller_bss_asm_op;

#define TEXT_SECTION_ASM_OP  propeller_text_asm_op
#define DATA_SECTION_ASM_OP  propeller_data_asm_op
#define BSS_SECTION_ASM_OP   propeller_bss_asm_op

/* call TARGET_ASM_SELECT_SECTION for functions as well as variables */
#define USE_SELECT_SECTION_FOR_FUNCTIONS 1

#define INIT_SECTION_ASM_OP \
  (TARGET_PASM ? "\t'init section\t" : "\tsection\t\".init\",\"ax\"")
#define GLOBAL_ASM_OP \
  (TARGET_PASM ? "\t'global variable\t" : "\t.global\t")
#define SET_ASM_OP \
  (TARGET_PASM ? "\t'set\t" : "\t.set\t")

/* For now put read-only data into the data section; that works
   on all current targets. If someday we get ROMable code we
   may want to revisit this. */
#define READONLY_DATA_SECTION_ASM_OP propeller_data_asm_op

#define TARGET_ASM_NAMED_SECTION default_elf_asm_named_section

#define ASM_DECLARE_FUNCTION_NAME(FILE, NAME, DECL) propeller_declare_function_name ((FILE), (NAME), (DECL))

/* Assembler Commands for Alignment */

#define ASM_OUTPUT_ALIGN(STREAM,POWER)                          \
  do { if (TARGET_PASM)                                         \
    {                                                           \
      if (POWER == 1)                                           \
        fprintf (STREAM, "\tword\n");                           \
      else                                                      \
        fprintf (STREAM, "\tlong\n");                           \
    }                                                           \
  else                                                          \
    fprintf (STREAM, "\t.balign\t%u\n", (1U<<POWER)); } while (0)

/* This says how to output an assembler line
   to define a global common symbol.  */

#define ASM_OUTPUT_COMMON(FILE, NAME, SIZE, ROUNDED)    \
  propeller_asm_output_aligned_common (STREAM, NULL_RTX, NAME, ROUNDED, 0, 1)

/* This says how to output an assembler line
   to define a local common symbol....  */
#undef  ASM_OUTPUT_LOCAL
#define ASM_OUTPUT_LOCAL(FILE, NAME, SIZE, ROUNDED)     \
  propeller_asm_output_aligned_common (STREAM, NULL_RTX, NAME, SIZE, 0, 1)


/* ... and how to define a local common symbol whose alignment
   we wish to specify.  ALIGN comes in as bits, we have to turn
   it into bytes.  */
#define ASM_OUTPUT_ALIGNED_DECL_COMMON(STREAM, DECL, NAME, SIZE, ALIGNMENT) \
  propeller_asm_output_aligned_common (STREAM, DECL, NAME, SIZE, ALIGNMENT, 1)
#define ASM_OUTPUT_ALIGNED_DECL_LOCAL(STREAM, DECL, NAME, SIZE, ALIGNMENT) \
  propeller_asm_output_aligned_common (STREAM, DECL, NAME, SIZE, ALIGNMENT, 0)

/* This is how to output an assembler line
   that says to advance the location counter by SIZE bytes.  */
#undef  ASM_OUTPUT_SKIP
#define ASM_OUTPUT_SKIP(FILE,SIZE)  \
  do {                                          \
  if (TARGET_PASM)                              \
    fprintf (FILE, "\tbyte 0[%d]\n", (int)(SIZE));      \
  else                                                  \
    fprintf (FILE, "\t.zero\t%d\n", (int)(SIZE));       \
  } while(0)

/* Output and Generation of Labels */

#define ASM_OUTPUT_LABEL(FILE,NAME) propeller_output_label(FILE,NAME)
#define ASM_OUTPUT_INTERNAL_LABEL(FILE,NAME) propeller_output_label(FILE,NAME)

#undef  ASM_GENERATE_INTERNAL_LABEL
#define ASM_GENERATE_INTERNAL_LABEL(LABEL, PREFIX, NUM)         \
  do                                                            \
    {                                                           \
      if (TARGET_PASM)                                          \
        sprintf (LABEL, "*L_%s%u",                              \
                 PREFIX, (unsigned) (NUM));                     \
      else                                                      \
        sprintf (LABEL, "*.%s%u", PREFIX, (unsigned) (NUM));    \
    }                                                           \
  while (0)

/* Store in OUTPUT a string (made with alloca) containing an
   assembler-name for a local static variable named NAME.  LABELNO is
   an integer which is different for each call.  PASM can't
   use periods to generate the name, so we use a ___ separator
   instead. */

#define ASM_FORMAT_PRIVATE_NAME(OUTPUT, NAME, LABELNO)  \
  do {                                                                  \
    (OUTPUT) = (char *) alloca (strlen ((NAME)) + 15);                  \
    sprintf ((OUTPUT), "%s.%lu", (NAME), (unsigned long)(LABELNO));     \
    if (TARGET_PASM) {                                                  \
      char *periodx;                                                    \
      for (periodx = (OUTPUT); *periodx; periodx++)                     \
        if (*periodx == '.') *periodx = '_';                            \
    }                                                                   \
  } while (0)

#define ASM_WEAKEN_LABEL(FILE,NAME) propeller_weaken_label(FILE,NAME)

#ifndef ASM_OUTPUT_EXTERNAL
#define ASM_OUTPUT_EXTERNAL(FILE, DECL, NAME) \
  default_elf_asm_output_external (FILE, DECL, NAME)
#endif

/* Debugging information */
#define PREFERRED_DEBUGGING_TYPE DWARF2_DEBUG
#define DWARF2_DEBUGGING_INFO 1
#define DWARF2_ASM_LINE_DEBUG_INFO 1

#define TYPE_ASM_OP  "\t.type\t"
#define SIZE_ASM_OP  "\t.size\t"
#define TYPE_OPERAND_FMT "@%s"

/* propeller specific defines */
#define SYMBOL_FLAG_PROPELLER_COGMEM (SYMBOL_FLAG_MACH_DEP << 0)

#define CONSTANT_POOL_BEFORE_FUNCTION (0)

#define SWITCHABLE_TARGET 1

#ifndef USED_FOR_TARGET
extern GTY(()) struct target_globals *propeller_cog_globals;
extern enum processor propeller_cpu;
extern int propeller_use_delay_slots;
#endif

/* Like REG_P except that this macro is true for SET expressions.  */
#define SET_P(rtl)    (GET_CODE (rtl) == SET)

#define DBR_OUTPUT_SEQEND(STREAM) propeller_output_seqend(STREAM)

#endif /* GCC_PROPELLER_H */
