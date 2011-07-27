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

;; Nonzero if OP is suitable as a general propeller source operand

(define_predicate "propeller_src_operand"
  (ior (match_operand 0 "register_operand")
       (and (match_operand 0 "immediate_operand")
            (match_test "propeller_const_ok_for_letter_p(INTVAL(op), 'I')")

)))

;; for addition we can also do negative immediates
(define_predicate "propeller_add_operand"
  (ior (match_operand 0 "register_operand")
       (and (match_operand 0 "immediate_operand")
            (ior
	        (match_test "propeller_const_ok_for_letter_p(INTVAL(op), 'I')")
	        (match_test "propeller_const_ok_for_letter_p(INTVAL(op), 'N')")

))))

;; Nonzero if OP can be source of a simple move operations
(define_predicate "propeller_movsrc_operand"
  (match_code "mem,const_int,reg,subreg,symbol_ref,label_ref,const")
{
  return general_operand (op, mode);
})
