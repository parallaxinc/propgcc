;; Machine description for Propeller
;; Copyright (C) 2011 Parallax, Inc.
;; Copyright (C) 2009 Free Software Foundation, Inc.
;; Contributed by Eric R. Smith <ersmith@totalspectrum.ca>
;; Based in part on the Moxie machine description
;;   contributed by Anthony Green <green@moxielogic.com>

;; This file is part of GCC.

;; GCC is free software; you can redistribute it and/or modify it
;; under the terms of the GNU General Public License as published
;; by the Free Software Foundation; either version 3, or (at your
;; option) any later version.

;; GCC is distributed in the hope that it will be useful, but WITHOUT
;; ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
;; or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
;; License for more details.

;; You should have received a copy of the GNU General Public License
;; along with GCC; see the file COPYING3.  If not see
;; <http://www.gnu.org/licenses/>.

;; -------------------------------------------------------------------------
;; Moxie specific constraints, predicates and attributes
;; -------------------------------------------------------------------------

(include "constraints.md")
(include "predicates.md")

; make sure this matches the definition in propeller.h
(define_constants
  [(CC_REG 20)])

; Most instructions are four bytes long.
(define_attr "length" "" (const_int 4))

;; -------------------------------------------------------------------------
;; nop instruction
;; -------------------------------------------------------------------------

(define_insn "nop"
  [(const_int 0)]
  ""
  "nop")

;; -------------------------------------------------------------------------
;; Arithmetic instructions
;; -------------------------------------------------------------------------

(define_insn "addsi3"
  [(set (match_operand:SI 0 "register_operand" "=r,r")
	  (plus:SI
	   (match_operand:SI 1 "register_operand" "0,0")
	   (match_operand:SI 2 "propeller_src_operand" "rI,N")))]
  ""
  "@
  add %0, %2
  sub %0, %2")

(define_insn "subsi3"
  [(set (match_operand:SI 0 "register_operand" "=r,r")
	  (minus:SI
	   (match_operand:SI 1 "register_operand" "0,0")
	   (match_operand:SI 2 "propeller_src_operand" "I,r")))]
  ""
  "@
  sub %0, %2
  sub %0, %2")

;; -------------------------------------------------------------------------
;; Unary arithmetic instructions
;; -------------------------------------------------------------------------

(define_insn "negsi2"
  [(set (match_operand:SI 0 "register_operand" "=r")
	  (neg:SI (match_operand:SI 1 "register_operand" "r")))]
  ""
  "neg    %0, %1")

(define_insn "one_cmplsi2"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(not:SI (match_operand:SI 1 "register_operand" "r")))]
  ""
  "not    %0, %1")

;; -------------------------------------------------------------------------
;; Logical operators
;; -------------------------------------------------------------------------

(define_insn "andsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(and:SI (match_operand:SI 1 "register_operand" "0")
		(match_operand:SI 2 "propeller_src_operand" "rI")))]
  ""
{
  return "and    %0, %2";
})

(define_insn "xorsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(xor:SI (match_operand:SI 1 "register_operand" "0")
		(match_operand:SI 2 "propeller_src_operand" "rI")))]
  ""
{
  return "xor    %0, %2";
})

(define_insn "iorsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(ior:SI (match_operand:SI 1 "register_operand" "0")
		(match_operand:SI 2 "propeller_src_operand" "rI")))]
  ""
{
  return "or     %0, %2";
})

;; -------------------------------------------------------------------------
;; Shifters
;; -------------------------------------------------------------------------

(define_insn "ashlsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(ashift:SI (match_operand:SI 1 "register_operand" "0")
		   (match_operand:SI 2 "propeller_src_operand" "rI")))]
  ""
{
  return "shl   %0, %2";
})

(define_insn "ashrsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(ashiftrt:SI (match_operand:SI 1 "register_operand" "0")
		     (match_operand:SI 2 "register_operand" "r")))]
  ""
{
  return "sar   %0, %2";
})

(define_insn "lshrsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(lshiftrt:SI (match_operand:SI 1 "register_operand" "0")
		     (match_operand:SI 2 "register_operand" "r")))]
  ""
{
  return "shr   %0, %2";
})

;; -------------------------------------------------------------------------
;; Move instructions
;; -------------------------------------------------------------------------

;; SImode

(define_expand "movsi"
   [(set (match_operand:SI 0 "general_operand" "")
 	(match_operand:SI 1 "general_operand" ""))]
   ""
  "
{
  /* If this is a store, force the value into a register.  */
  if (! (reload_in_progress || reload_completed))
  {
    if (MEM_P (operands[0]))
    {
      operands[1] = force_reg (SImode, operands[1]);
      if (MEM_P (XEXP (operands[0], 0)))
        operands[0] = gen_rtx_MEM (SImode, force_reg (SImode, XEXP (operands[0], 0)));
    }
    else 
      if (MEM_P (operands[1])
          && MEM_P (XEXP (operands[1], 0)))
        operands[1] = gen_rtx_MEM (SImode, force_reg (SImode, XEXP (operands[1], 0)));
  }
}")

(define_insn "*movsi"
  [(set (match_operand:SI 0 "general_operand"          "=r,r,r,W")
	(match_operand:SI 1 "propeller_movsrc_operand" "rI,N,W,r"))]
  "register_operand (operands[0], SImode)
   || register_operand (operands[1], SImode)"
  "@
   mov    %0, %1
   neg    %0, %1
   rdlong %0, %1
   wrlong %1, %0")

(define_expand "movqi"
  [(set (match_operand:QI 0 "general_operand" "")
	(match_operand:QI 1 "general_operand" ""))]
  ""
  "
{
  /* If this is a store, force the value into a register.  */
  if (MEM_P (operands[0]))
    operands[1] = force_reg (QImode, operands[1]);
}")

(define_insn "*movqi"
  [(set (match_operand:QI 0 "general_operand"          "=r,r,r,W")
	(match_operand:QI 1 "propeller_movsrc_operand" "rI,N,W,r"))]
  "register_operand (operands[0], QImode)
   || register_operand (operands[1], QImode)"
  "@
   mov    %0, %1
   neg    %0, %1
   rdbyte %0, %1
   wrbyte %1, %0")


(define_expand "movhi"
  [(set (match_operand:HI 0 "general_operand" "")
	(match_operand:HI 1 "general_operand" ""))]
  ""
  "
{
  /* If this is a store, force the value into a register.  */
  if (MEM_P (operands[0]))
    operands[1] = force_reg (HImode, operands[1]);
}")

(define_insn "*movhi"
  [(set (match_operand:HI 0 "general_operand"          "=r,r,r,W")
	(match_operand:HI 1 "propeller_movsrc_operand" "rI,N,W,r"))]
  "register_operand (operands[0], HImode)
   || register_operand (operands[1], HImode)"
  "@
   mov    %0, %1
   neg    %0, %1
   rdbyte %0, %1
   wrbyte %1, %0")

;; -------------------------------------------------------------------------
;; Compare instructions
;; -------------------------------------------------------------------------

(define_expand "cbranchsi4"
  [(set (reg:CC CC_REG)
        (compare:CC
         (match_operand:SI 1 "general_operand" "")
         (match_operand:SI 2 "general_operand" "")))
   (set (pc)
        (if_then_else (match_operator:CC 0 "comparison_operator"
                       [(reg:CC CC_REG) (const_int 0)])
                      (label_ref (match_operand 3 "" ""))
                      (pc)))]
  ""
  "
  /* Force the compare operands into registers.  */
  if (GET_CODE (operands[1]) != REG)
	operands[1] = force_reg (SImode, operands[1]);
  if (GET_CODE (operands[2]) != REG)
	operands[2] = force_reg (SImode, operands[2]);
  ")

(define_insn "*cmpsi"
  [(set (reg:CC CC_REG)
	(compare
	 (match_operand:SI 0 "register_operand" "r")
	 (match_operand:SI 1 "register_operand"	"r")))]
  ""
  "cmp    %0, %1")


;; -------------------------------------------------------------------------
;; Branch instructions
;; -------------------------------------------------------------------------

(define_code_iterator cond [ne eq lt ltu gt gtu ge le geu leu])
(define_code_attr CC [(ne "ne") (eq "eq") (lt "lt") (ltu "ltu") 
		      (gt "gt") (gtu "gtu") (ge "ge") (le "le") 
		      (geu "geu") (leu "leu") ])
(define_code_attr rCC [(ne "eq") (eq "ne") (lt "ge") (ltu "geu") 
		       (gt "le") (gtu "leu") (ge "lt") (le "gt") 
		       (geu "ltu") (leu "gtu") ])

(define_insn "*b<cond:code>"
  [(set (pc)
	(if_then_else (cond (reg:CC CC_REG)
			    (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
{
  return "IF_<CC> jmp   %l0";
})


;; -------------------------------------------------------------------------
;; Call and Jump instructions
;; -------------------------------------------------------------------------

(define_expand "call"
  [(call (match_operand:QI 0 "memory_operand" "")
		(match_operand 1 "general_operand" ""))]
  ""
{
  gcc_assert (MEM_P (operands[0]));
})

(define_insn "*call"
  [(call (mem:QI (match_operand:SI
		  0 "nonmemory_operand" "ir"))
	 (match_operand 1 "" ""))]
  ""
  "@
   jmpret   lr,%0"
)

(define_expand "call_value"
  [(set (match_operand 0 "" "")
		(call (match_operand:QI 1 "memory_operand" "")
		 (match_operand 2 "" "")))]
  ""
{
  gcc_assert (MEM_P (operands[1]));
})

(define_insn "*call_value"
  [(set (match_operand 0 "register_operand" "=r")
	(call (mem:QI (match_operand:SI
		       1 "immediate_operand" "i"))
	      (match_operand 2 "" "")))]
  ""
  "jmpret   lr,%1"
 )

(define_insn "*call_value_indirect"
  [(set (match_operand 0 "register_operand" "=r")
	(call (mem:QI (match_operand:SI
		       1 "register_operand" "r"))
	      (match_operand 2 "" "")))]
  ""
  "jmpret   lr,%1")

(define_insn "indirect_jump"
  [(set (pc) (match_operand:SI 0 "nonimmediate_operand" "r"))]
  ""
  "jmp    %0")

(define_insn "jump"
  [(set (pc)
	(label_ref (match_operand 0 "" "")))]
  ""
  "jmp   %0")


;; -------------------------------------------------------------------------
;; Prologue & Epilogue
;; -------------------------------------------------------------------------

(define_expand "prologue"
  [(clobber (const_int 0))]
  ""
  "
{
  propeller_expand_prologue ();
  DONE;
}
")

(define_expand "epilogue"
  [(return)]
  ""
  "
{
  propeller_expand_epilogue ();
  DONE;
}
")

(define_insn "return_internal"
  [(use (match_operand:SI 0 "register_operand" "r"))
   (return)]
  ""
  "jmp        %0"
)

(define_insn "return"
  [(return)]
  "propeller_can_use_return ()"
  "jmp lr"
)
