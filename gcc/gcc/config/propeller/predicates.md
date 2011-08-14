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

(define_special_predicate "cc_register"
  (and (match_code "reg")
       (and (match_test "REGNO (op) == PROP_CC_REGNUM")
	    (ior (match_test "mode == GET_MODE (op)")
		 (match_test "mode == VOIDmode && GET_MODE_CLASS (GET_MODE (op)) == MODE_CC")))))

;; Nonzero if OP is a 32 bit constant that needs to be placed specially

(define_predicate "propeller_big_const"
  (and (match_operand 0 "immediate_operand")
       (match_test "propeller_const_ok_for_letter_p(INTVAL(op), 'W')"
)))

(define_predicate "immediate_1_9"
  (and (match_code "const_int")
       (match_test "IN_RANGE (INTVAL (op), 1, 9)")))
