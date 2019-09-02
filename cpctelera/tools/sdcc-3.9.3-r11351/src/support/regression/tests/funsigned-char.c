/*
 * check for the correct signness of a char constant
 *   (signed versus unsigned)
 *   (indirect via integer promotion)
 *
 * Note, the check for the --funsigned-char must be invoked by hand
 *   see the following emacs sexp
 *     (compile "SDCCFLAGS=--funsigned-char make -C .. ALL_TESTS='./tests/funsigned-char.c'")
 *     (compile "make -C .. ALL_TESTS='./tests/funsigned-char.c'")
 *
 */

#include <testfwk.h>
#include <stdint.h>
#include <limits.h>

int glb_schar_to_int = ~ (signed char)   '\200';
int glb_uchar_to_int = ~ (unsigned char) '\200';
int glb_char_to_int  = ~                 '\200';

int tst_schar_to_int()  { return ~ (signed char)   '\200'; }
int tst_uchar_to_int()  { return ~ (unsigned char) '\200'; }
int tst_char_to_int()   { return ~                 '\200'; }


void
testBug(void)
{
#if CHAR_MIN >= 0
  ASSERT(CHAR_MAX ==  255);
  ASSERT(CHAR_MIN ==    0);
#else
  ASSERT(CHAR_MAX ==  127);
  ASSERT(CHAR_MIN == -128);
#endif

  ASSERT(tst_uchar_to_int() == -129);
  ASSERT(glb_uchar_to_int   == -129);

  ASSERT(tst_schar_to_int() ==  127);
  ASSERT(glb_schar_to_int   ==  127);

#if CHAR_MIN >= 0
  ASSERT(tst_char_to_int() == -129);
  ASSERT(glb_char_to_int   == -129);
#else
  ASSERT(tst_char_to_int() ==  127);
  ASSERT(glb_char_to_int   ==  127);
#endif
}

