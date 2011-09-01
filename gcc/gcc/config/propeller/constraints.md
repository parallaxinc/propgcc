;; Constraint definitions for Propeller
;; Copyright (C) 2011 Parallax, Inc.
;; Copyright (C) 2009 Free Software Foundation, Inc.
;; Contributed by Eric R. Smith <ersmith@totalspectrum.ca>
;; Based on the Moxie constraints.md
;;    contributed by Anthony Green <green@moxielogic.com>

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
;; Constraints
;; -------------------------------------------------------------------------

(define_constraint "A"
  "An absolute address."
  (and (match_code "mem")
       (ior (match_test "GET_CODE (XEXP (op, 0)) == SYMBOL_REF")
	    (match_test "GET_CODE (XEXP (op, 0)) == LABEL_REF")
	    (match_test "GET_CODE (XEXP (op, 0)) == CONST"))))

(define_constraint "B"
  "A cog memory address"
  (match_test "propeller_cogaddr_p (op)"))

;;
;; note that a cog memory reference is not actually a "memory_constraint"
;; as gcc uses the term, because we can't access it via a base register
;;
(define_constraint "C"
  "A cog memory reference"
  (and (match_code "mem")
       (match_test "propeller_cogmem_p (op)")))

(define_memory_constraint "Q"
  "A register or cog memory indirect memory operand."
  (and (match_code "mem")
       (ior (match_test "REG_P (XEXP (op,0))
                    && REGNO_OK_FOR_BASE_P (REGNO (XEXP (op,0)))")
	    (match_test "propeller_cogmem_p (XEXP (op,0))"))))

(define_memory_constraint "h"
  "A memory address known not to be in cog RAM"
  (and (match_code "mem")
       (match_test "!propeller_cogmem_p (op)")))

(define_constraint "O"
  "The constant zero"
  (and (match_code "const_int")
       (match_test "ival == 0")))

(define_constraint "I"
  "A 9-bit constant (0..511)"
  (and (match_code "const_int")
       (match_test "ival >= 0 && ival <= 511")))

(define_constraint "M"
  "A complemented 9-bit constant ~(0..511)"
  (and (match_code "const_int")
       (match_test "ival >= -512 && ival < 0")))

(define_constraint "N"
  "A negative 9-bit constant -(0..511)"
  (and (match_code "const_int")
       (match_test "ival >= -511 && ival <= 0")))

(define_constraint "W"
  "A wide integer (does not fit in 9 bits)"
  (and (match_code "const_int")
       (match_test "ival < 0 && ival >= 511")))

;;
;; some register constraints
;;
(define_register_constraint "z" "R0_REGS"
  "R0 register")

(define_register_constraint "y" "R1_REGS"
  "R1 register")
