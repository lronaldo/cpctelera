/*
   bug-136564.c0

   loop induction
*/

#include <testfwk.h>


void
testBackPatchLabel(void)
{
  volatile unsigned char c0 = 0, c1 = 1;
  unsigned char r;

  if (     (c0 == 0)) r = 1; else r = 0; ASSERT(r == 1);
  if (    !(c0 == 0)) r = 1; else r = 0; ASSERT(r == 0);
  if (   !!(c0 == 0)) r = 1; else r = 0; ASSERT(r == 1);
  if (  !!!(c0 == 0)) r = 1; else r = 0; ASSERT(r == 0);
  if ( !!!!(c0 == 0)) r = 1; else r = 0; ASSERT(r == 1);
  if (!!!!!(c0 == 0)) r = 1; else r = 0; ASSERT(r == 0);

  if (     ((c0 == 0) && (c1 == 1))) r = 1; else r = 0; ASSERT(r == 1);
  if (    !((c0 == 0) && (c1 == 1))) r = 1; else r = 0; ASSERT(r == 0);
  if (   !!((c0 == 0) && (c1 == 1))) r = 1; else r = 0; ASSERT(r == 1);

  if (     (  (c0 == 0) &&   (c1 == 1))) r = 1; else r = 0; ASSERT(r == 1);
  if (    !( !(c0 == 1) &&  !(c1 == 0))) r = 1; else r = 0; ASSERT(r == 0);
  if (   !!(!!(c0 == 0) && !!(c1 == 1))) r = 1; else r = 0; ASSERT(r == 1);
}
