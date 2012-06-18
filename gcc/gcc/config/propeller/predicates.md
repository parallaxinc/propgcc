;; Predicate definitions for Propeller
;; Copyright (C) 2011 Parallax, Inc.
;; Copyright (C) 2009 Free Software Foundation, Inc.
;; Contributed by Eric R. Smith <ersmith@totalspectrum.ca>
;; Based in part on the Moxie predicate definition file
;; contributed by Anthony Green <green@moxielogic.com>

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
;; Predicates
;; -------------------------------------------------------------------------

;; True if OP is a valid operand for the MEM of a CALL insn.
(define_predicate "call_operand"
  (ior (match_code "symbol_ref")
       (match_operand 0 "register_operand")))

;; True if OP refers to any kind of symbol.
;; For roughly the same reasons that pmode_register_operand exists, this
;; predicate ignores its mode argument.
(define_special_predicate "symbolic_operand" 
   (match_code "symbol_ref,const,label_ref"))

;; True if OP is a SYMBOL_REF which refers to a function.
(define_predicate "function_operand"
  (and (match_code "symbol_ref")
       (match_test "SYMBOL_REF_FUNCTION_P (op)")))

;; True if OP is suitable as a general propeller destination operand
(define_predicate "propeller_dst_operand"
  (ior (match_operand 0 "register_operand")
       (and (match_operand 0 "memory_operand")
            (match_test "propeller_cogmem_p (op)"))))

;; True if OP is suitable as a register propeller destination operand
;; in TARGET_XMM the stack pointer and other cog memory locations
;; are not a suitable destination for
;; loads, otherwise this is the same as propeller_dst_operand

(define_predicate "propeller_reg_operand"
  (ior (and (match_operand 0 "register_operand")
            (match_test "!TARGET_XMM || REGNO (op) != PROP_SP_REGNUM"))
       (and (match_operand 0 "memory_operand")
            (match_test "!TARGET_XMM && propeller_cogmem_p (op)"))))


;; Nonzero if OP is suitable as a general propeller source operand

(define_predicate "propeller_src_operand"
  (ior (match_operand 0 "propeller_dst_operand")
       (and (match_operand 0 "immediate_operand")
            (match_test "propeller_const_ok_for_letter_p(INTVAL(op), 'I')"))
))

;; for addition we can also do negative immediates
(define_predicate "propeller_add_operand"
  (ior (match_operand 0 "propeller_src_operand")
       (and (match_operand 0 "immediate_operand")
	    (match_test "propeller_const_ok_for_letter_p(INTVAL(op), 'N')"))

))

;; for and we can do "andn" of immediates
(define_predicate "propeller_and_operand"
  (ior (match_operand 0 "propeller_src_operand")
       (and (match_operand 0 "immediate_operand")
            (match_test "propeller_const_ok_for_letter_p(INTVAL(op), 'M')"))))


;; True if this operator is valid for predication
(define_special_predicate "predicate_operator"
  (match_code "eq,ne,le,lt,ge,gt,geu,gtu,leu,ltu"))

;; true if this operator is a math operator with 2 arguments
;; note that we skip smin,smax,umin,umax because
;; they do not set flags the way the others do

(define_special_predicate "propeller_math_op2"
  (match_code "plus,minus,and,ior,xor"))

;; true if this operator is a math operator with 1 arguments
(define_special_predicate "propeller_math_op1"
  (match_code "neg,not,abs"))

;; match the cc register
(define_special_predicate "cc_register"
  (and (match_code "reg")
       (and (match_test "REGNO (op) == PROP_CC_REGNUM")
	    (ior (match_test "mode == GET_MODE (op)")
		 (match_test "mode == VOIDmode && GET_MODE_CLASS (GET_MODE (op)) == MODE_CC")))))

;; Nonzero if OP is a 32 bit constant that needs to be placed specially

(define_predicate "propeller_big_const"
  (and (match_operand 0 "immediate_operand")
       (ior (match_code "symbol_ref,label_ref,const")
            (and (match_code "const_int")
	         (match_test "!IN_RANGE (INTVAL (op), -511, 511)")))))

(define_predicate "immediate_1_9"
  (and (match_code "const_int")
       (match_test "IN_RANGE (INTVAL (op), 1, 9)")))

(define_predicate "immediate_0_8"
  (and (match_code "const_int")
       (match_test "IN_RANGE (INTVAL (op), 0, 8)")))

;;
;; true for an operand that we know is on the stack
;;
(define_predicate "stack_operand"
  (and (match_code "mem")
       (match_test "propeller_stack_operand_p (op)")))

;;
;; Next to predicates are taken from the RX machine description
;;

;; Return true if OP is a store multiple operation.  This looks like:
;;
;;   [(set (SP) (MINUS (SP) (INT)))
;;    (set (MEM (SP)) (REG))
;;    (set (MEM (MINUS (SP) (INT))) (REG)) {optionally repeated}
;;   ]

(define_special_predicate "propeller_store_multiple_vector"
  (match_code "parallel")
{
  int count = XVECLEN (op, 0);
  unsigned int src_regno;
  rtx element;
  int i;

  /* Perform a quick check so we don't blow up below.  */
  if (count <= 2)
    return false;

  /* Check that the first element of the vector is the stack adjust.  */
  element = XVECEXP (op, 0, 0);
  if (   ! SET_P (element)
      || ! REG_P (SET_DEST (element))
      ||   REGNO (SET_DEST (element)) != SP_REG
      ||   GET_CODE (SET_SRC (element)) != MINUS
      || ! REG_P (XEXP (SET_SRC (element), 0))
      ||   REGNO (XEXP (SET_SRC (element), 0)) != SP_REG
      || ! CONST_INT_P (XEXP (SET_SRC (element), 1)))
    return false;
	 
  /* Check that the next element is the first push.  */
  element = XVECEXP (op, 0, 1);
  if (   ! SET_P (element)
      || ! REG_P (SET_SRC (element))
      || GET_MODE (SET_SRC (element)) != SImode
      || ! MEM_P (SET_DEST (element))
      || GET_MODE (SET_DEST (element)) != SImode
      || GET_CODE (XEXP (SET_DEST (element), 0)) != MINUS
      || ! REG_P (XEXP (XEXP (SET_DEST (element), 0), 0))
      ||   REGNO (XEXP (XEXP (SET_DEST (element), 0), 0)) != SP_REG
      || ! CONST_INT_P (XEXP (XEXP (SET_DEST (element), 0), 1))
      || INTVAL (XEXP (XEXP (SET_DEST (element), 0), 1))
        != GET_MODE_SIZE (SImode))
    return false;

  src_regno = REGNO (SET_SRC (element));

  /* Check that the remaining elements use SP-<disp>
     addressing and increasing register numbers.  */
  for (i = 2; i < count; i++)
    {
      element = XVECEXP (op, 0, i);

      if (   ! SET_P (element)
	  || ! REG_P (SET_SRC (element))
	  || GET_MODE (SET_SRC (element)) != SImode
	  || REGNO (SET_SRC (element)) != src_regno + (i - 1)
	  || ! MEM_P (SET_DEST (element))
	  || GET_MODE (SET_DEST (element)) != SImode
	  || GET_CODE (XEXP (SET_DEST (element), 0)) != MINUS
          || ! REG_P (XEXP (XEXP (SET_DEST (element), 0), 0))
          ||   REGNO (XEXP (XEXP (SET_DEST (element), 0), 0)) != SP_REG
	  || ! CONST_INT_P (XEXP (XEXP (SET_DEST (element), 0), 1))
	  || INTVAL (XEXP (XEXP (SET_DEST (element), 0), 1))
	     != i * GET_MODE_SIZE (SImode))
	return false;
    }
  return true;
})

;; Return true if OP is a load multiple operation.
;; This looks like:
;;  [(set (SP) (PLUS (SP) (INT)))
;;   (set (REG) (MEM (SP)))
;;   (set (REG) (MEM (PLUS (SP) (INT)))) {optionally repeated}
;;  ]

(define_special_predicate "propeller_load_multiple_vector"
  (match_code "parallel")
{
  int count = XVECLEN (op, 0);
  unsigned int dest_regno;
  rtx element;
  int i;

  /* Perform a quick check so we don't blow up below.  */
  if (count <= 2)
    return false;

  /* Check that the first element of the vector is the stack adjust.  */
  element = XVECEXP (op, 0, 0);
  if (   ! SET_P (element)
      || ! REG_P (SET_DEST (element))
      ||   REGNO (SET_DEST (element)) != SP_REG
      ||   GET_CODE (SET_SRC (element)) != PLUS
      || ! REG_P (XEXP (SET_SRC (element), 0))
      ||   REGNO (XEXP (SET_SRC (element), 0)) != SP_REG
      || ! CONST_INT_P (XEXP (SET_SRC (element), 1)))
    return false;
	 
  /* Check that the next element is the first push.  */
  element = XVECEXP (op, 0, 1);
  if (   ! SET_P (element)
      || ! REG_P (SET_DEST (element))
      || ! MEM_P (SET_SRC (element))
      || ! REG_P (XEXP (SET_SRC (element), 0))
      ||   REGNO (XEXP (SET_SRC (element), 0)) != SP_REG)
    return false;

  dest_regno = REGNO (SET_DEST (element));

  /* Check that the remaining elements use SP+<disp>
     addressing and decremental register numbers.  */
  for (i = 2; i < count; i++)
    {
      element = XVECEXP (op, 0, i);

      if (   ! SET_P (element)
	  || ! REG_P (SET_DEST (element))
	  || GET_MODE (SET_DEST (element)) != SImode
	  || REGNO (SET_DEST (element)) != dest_regno - (i - 1)
	  || ! MEM_P (SET_SRC (element))
	  || GET_MODE (SET_SRC (element)) != SImode
	  || GET_CODE (XEXP (SET_SRC (element), 0)) != PLUS
          || ! REG_P (XEXP (XEXP (SET_SRC (element), 0), 0))
          ||   REGNO (XEXP (XEXP (SET_SRC (element), 0), 0)) != SP_REG
	  || ! CONST_INT_P (XEXP (XEXP (SET_SRC (element), 0), 1))
	  || INTVAL (XEXP (XEXP (SET_SRC (element), 0), 1))
	     != (i - 1) * GET_MODE_SIZE (SImode))
	return false;
    }
  return true;
})
