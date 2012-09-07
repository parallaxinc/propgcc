/**
 * @file include/iso646.h
 * @brief Provides macros for keyboards not having equivalent symbols.
 *
 * The iso646.h header file is part of the C standard library. It was
 * added to this library in a 1995 amendment to the C90 standard.
 * It defines a number of macros which allow programmers to use C
 * language bitwise and logical operators, which, without the header
 * file, cannot be quickly or easily typed on some international and
 * non-QWERTY keyboards.
 */
#ifndef _ISO646_H
#define _ISO646_H

#define and    &&
#define and_eq &=
#define bitand &
#define bitor  |
#define compl  ~
#define not    !
#define not_eq !=
#define or     ||
#define or_eq  |=
#define xor    ^
#define xor_eq ^=

#endif
