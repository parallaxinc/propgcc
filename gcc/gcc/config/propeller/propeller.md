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
;; Propeller specific constraints, predicates and attributes
;; -------------------------------------------------------------------------

; make sure this matches the definition in propeller.h
(define_constants
  [(CC_REG 18)])

; Most instructions are four bytes long.
(define_attr "length" "" (const_int 4))

(include "constraints.md")
(include "predicates.md")

;;
;; instruction types
;; core == normal instruction
;; hub  == instruction that references hub memory
;; multi == an insn that expands to multiple instructions
;;
(define_attr "type" "core,hub,multi" (const_string "core"))


;; -------------------------------------------------------------------------
;; machine model for instruction scheduling
;; the tricky part here is that hub memory operations are only available
;; every 16 cycles (4 instructions) and that 16 cycle period is independent
;; of what's going on inside the processor -- if we miss a hub window we
;; have to wait for the next one to come along
;;
;; we model this by pretending there are 4 slots; core operations issue
;; to any of the slots, hub operations can only issue to slot1
;; -------------------------------------------------------------------------
(define_cpu_unit "issue,slot1,slot2,slot3,slot4")

(define_reservation "use_slot1" "(issue+slot1),slot1*3")
(define_reservation "use_slot2" "(issue+slot2),slot2*3")
(define_reservation "use_slot3" "(issue+slot3),slot3*3")
(define_reservation "use_slot4" "(issue+slot4),slot4*3")

(define_insn_reservation "coreop" 1 (eq_attr "type" "core")
			 "use_slot1 | use_slot2 | use_slot3 | use_slot4")
(define_insn_reservation "hubop" 1 (eq_attr "type" "hub")
			 "(issue+slot1+slot2),(slot1+slot2)*3")
(define_insn_reservation "multiop" 1 (eq_attr "type" "multi")
			 "issue+slot1,nothing*3")

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
	   (match_operand:SI 2 "propeller_add_operand" "rI,N")))]
  ""
  "@
  add\t%0, %2
  sub\t%0, #%n2")

(define_insn "subsi3"
  [(set (match_operand:SI 0 "register_operand" "=r,r")
	  (minus:SI
	   (match_operand:SI 1 "register_operand" "0,0")
	   (match_operand:SI 2 "propeller_src_operand" "I,r")))]
  ""
  "@
  sub\t%0, %2
  sub\t%0, %2")

;; -------------------------------------------------------------------------
;; Unary arithmetic instructions
;; -------------------------------------------------------------------------

(define_insn "negsi2"
  [(set (match_operand:SI 0 "register_operand" "=r")
	  (neg:SI (match_operand:SI 1 "register_operand" "r")))]
  ""
  "neg\t%0, %1")

(define_insn "one_cmplsi2"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(not:SI (match_operand:SI 1 "register_operand" "r")))]
  ""
  "not\t%0, %1")

;; -------------------------------------------------------------------------
;; Logical operators
;; -------------------------------------------------------------------------

(define_insn "andsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(and:SI (match_operand:SI 1 "register_operand" "0")
		(match_operand:SI 2 "propeller_src_operand" "rI")))]
  ""
{
  return "and\t%0, %2";
})

(define_insn "xorsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(xor:SI (match_operand:SI 1 "register_operand" "0")
		(match_operand:SI 2 "propeller_src_operand" "rI")))]
  ""
{
  return "xor\t%0, %2";
})

(define_insn "iorsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(ior:SI (match_operand:SI 1 "register_operand" "0")
		(match_operand:SI 2 "propeller_src_operand" "rI")))]
  ""
{
  return "or\t%0, %2";
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
  return "shl\t%0, %2";
})

(define_insn "ashrsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(ashiftrt:SI (match_operand:SI 1 "register_operand" "0")
		     (match_operand:SI 2 "propeller_src_operand" "rI")))]
  ""
{
  return "sar\t%0, %2";
})

(define_insn "lshrsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(lshiftrt:SI (match_operand:SI 1 "register_operand" "0")
		     (match_operand:SI 2 "propeller_src_operand" "rI")))]
  ""
{
  return "shr\t%0, %2";
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
  [(set (match_operand:SI 0 "nonimmediate_operand"           "=r,r,r,Q")
	(match_operand:SI 1 "general_operand"               "BrI,N,Q,r"))]
  "register_operand (operands[0], SImode)
   || register_operand (operands[1], SImode)"
  "@
   mov\t%0, %1
   neg\t%0, #%n1
   rdlong\t%0, %1
   wrlong\t%1, %0"
   [(set_attr "type" "core,core,hub,hub")]
)

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
  [(set (match_operand:QI 0 "nonimmediate_operand"   "=r,r,r,Q")
	(match_operand:QI 1 "general_operand"        "rI,N,Q,r"))]
  "register_operand (operands[0], QImode)
   || register_operand (operands[1], QImode)"
  "@
   mov\t%0, %1
   neg\t%0, #%n1
   rdbyte\t%0, %1
   wrbyte\t%1, %0"
   [(set_attr "type" "core,core,hub,hub")]
)


(define_expand "movhi"
  [(set (match_operand:HI 0 "nonimmediate_operand" "")
	(match_operand:HI 1 "general_operand" ""))]
  ""
  "
{
  /* If this is a store, force the value into a register.  */
  if (MEM_P (operands[0]))
    operands[1] = force_reg (HImode, operands[1]);
}")

(define_insn "*movhi"
  [(set (match_operand:HI 0 "nonimmediate_operand"          "=r,r,r,Q")
	(match_operand:HI 1 "general_operand" "rI,N,Q,r"))]
  "register_operand (operands[0], HImode)
   || register_operand (operands[1], HImode)"
  "@
   mov\t%0, %1
   neg\t%0, #%n1
   rdbyte\t%0, %1
   wrbyte\t%1, %0"
   [(set_attr "type" "core,core,hub,hub")]
)

;; optimizations
(define_insn "*prop_zero_extendqisi2"
  [(set (match_operand:SI 0 "register_operand" "=r,r")
	(zero_extend:SI (match_operand:QI 1 "nonimmediate_operand" "0,m")))]
  ""
  "@
   and\\t%0,#255
   rdbyte\\t%0 %1"
   [(set_attr "type" "core,hub")]
)

;; -------------------------------------------------------------------------
;; min/max instructions
;; -------------------------------------------------------------------------
(define_insn "umaxsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(umax:SI (match_operand:SI 1 "register_operand" "0")
		 (match_operand:SI 2 "propeller_src_operand" "rI")))]
  ""
  "max\\t%0, %2")

(define_insn "uminsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(umin:SI (match_operand:SI 1 "register_operand" "0")
		 (match_operand:SI 2 "propeller_src_operand" "rI")))]
  ""
  "min\\t%0, %2")

(define_insn "smaxsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(smax:SI (match_operand:SI 1 "register_operand" "0")
		 (match_operand:SI 2 "propeller_src_operand" "rI")))]
  ""
  "maxs\\t%0, %2")

(define_insn "sminsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(smin:SI (match_operand:SI 1 "register_operand" "0")
		 (match_operand:SI 2 "propeller_src_operand" "rI")))]
  ""
  "mins\\t%0, %2")

;; -------------------------------------------------------------------------
;; Compare instructions
;; -------------------------------------------------------------------------

(define_expand "cbranchsi4"
  [(set (match_dup 4)
        (match_op_dup 5
         [(match_operand:SI 1 "register_operand" "")
          (match_operand:SI 2 "propeller_src_operand" "")]))
   (set (pc)
        (if_then_else
              (match_operator 0 "ordered_comparison_operator"
               [(match_dup 4)
                (const_int 0)])
              (label_ref (match_operand 3 "" ""))
              (pc)))]
  ""
  "
{
  operands[4] = propeller_gen_compare_reg (GET_CODE (operands[0]),
                                      operands[1], operands[2]);
  operands[5] = gen_rtx_fmt_ee (COMPARE,
                                GET_MODE (operands[4]),
                                operands[1], operands[2]);
}")

(define_insn "*cmps"
  [(set (reg:CC CC_REG)
	(compare
	 (match_operand:SI 0 "register_operand" "r")
	 (match_operand:SI 1 "propeller_src_operand"	"rI")))]
  ""
  "cmps\t%0, %1")

(define_insn "*cmpu"
  [(set (reg:CCUNS CC_REG)
	(compare
	 (match_operand:SI 0 "register_operand" "r")
	 (match_operand:SI 1 "propeller_src_operand"	"rI")))]
  ""
  "cmp\t%0, %1")

;; -------------------------------------------------------------------------
;; Branch instructions
;; -------------------------------------------------------------------------

(define_code_iterator cond [ne eq lt ltu gt gtu ge le geu leu])
(define_code_attr CC [(ne "NE") (eq "E ") (lt "B ") (ltu "B ") 
		      (gt "A ") (gtu "A ") (ge "AE") (le "BE") 
		      (geu "AE") (leu "BE") ])

(define_insn "*b<cond:code>"
  [(set (pc)
	(if_then_else (cond (reg CC_REG)
			    (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
{
  return "IF_<CC> jmp\t%l0";
})


;; -------------------------------------------------------------------------
;; Call and Jump instructions
;; -------------------------------------------------------------------------

(define_expand "call"
  [(call (match_operand:SI 0 "memory_operand" "")
		(match_operand 1 "general_operand" ""))]
  ""
{
  gcc_assert (MEM_P (operands[0]));
})

(define_insn "*call"
  [(call (mem:SI (match_operand:SI
		  0 "nonmemory_operand" "ir"))
	 (match_operand 1 "" ""))]
  ""
  "@
   jmpret\tlr,%0"
)

(define_expand "call_value"
  [(set (match_operand 0 "" "")
		(call (match_operand:SI 1 "memory_operand" "")
		 (match_operand 2 "" "")))]
  ""
{
  gcc_assert (MEM_P (operands[1]));
})

(define_insn "*call_value"
  [(set (match_operand 0 "register_operand" "=r")
	(call (mem:SI (match_operand:SI
		       1 "immediate_operand" "i"))
	      (match_operand 2 "" "")))]
  ""
  "jmpret\tlr,%1"
 )

(define_insn "*call_value_indirect"
  [(set (match_operand 0 "register_operand" "=r")
	(call (mem:SI (match_operand:SI
		       1 "register_operand" "r"))
	      (match_operand 2 "" "")))]
  ""
  "jmpret\tlr,%1")

(define_insn "indirect_jump"
  [(set (pc) (match_operand:SI 0 "nonimmediate_operand" "r"))]
  ""
  "jmp\t%0")

(define_insn "jump"
  [(set (pc)
	(label_ref (match_operand 0 "" "")))]
  ""
  "jmp\t%0")


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
  "jmp\t%0"
)

(define_insn "return"
  [(return)]
  "propeller_can_use_return ()"
  "jmp\tlr"
)
