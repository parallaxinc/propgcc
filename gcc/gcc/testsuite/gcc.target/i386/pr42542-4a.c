/* { dg-do compile } */
/* { dg-options "-O1 -msse4.2 -ftree-vectorize" } */

#include "pr42542-4.c"

/* { dg-final { scan-assembler "pcmpgtq" } } */
