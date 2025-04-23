/*
   pr37924.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR c/37924 */


signed char a;
unsigned char b;

#if !(defined (__GNUC__) && defined (__GNUC_MINOR__) && (__GNUC__ < 5 && __GNUC_MINOR__ < 4))
int
ftest1 (void)
{
  int c = -1;
  return ((unsigned int) (a ^ c)) >> 9;
}

int
ftest2 (void)
{
  int c = -1;
  return ((unsigned int) (b ^ c)) >> 9;
}
#endif

void
testTortureExecute (void)
{
#if !(defined (__GNUC__) && defined (__GNUC_MINOR__) && (__GNUC__ < 5 && __GNUC_MINOR__ < 4))
  a = 0;
  if (ftest1 () != (-1U >> 9))
    ASSERT (0);
  a = 0x40;
  if (ftest1 () != (-1U >> 9))
    ASSERT (0);
  a = 0x80;
  if (ftest1 () != (a < 0) ? 0 : (-1U >> 9))
    ASSERT (0);
  a = 0xff;
  if (ftest1 () != (a < 0) ? 0 : (-1U >> 9))
    ASSERT (0);
  b = 0;
  if (ftest2 () != (-1U >> 9))
    ASSERT (0);
  b = 0x40;
  if (ftest2 () != (-1U >> 9))
    ASSERT (0);
  b = 0x80;
  if (ftest2 () != (-1U >> 9))
    ASSERT (0);
  b = 0xff;
  if (ftest2 () != (-1U >> 9))
    ASSERT (0);
  return;
#endif
}

