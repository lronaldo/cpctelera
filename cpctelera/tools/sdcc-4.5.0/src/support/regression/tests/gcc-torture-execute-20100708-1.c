/*
20100708-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#ifdef __SDCC
#pragma disable_warning 93
#endif

/* PR middle-end/44843 */
/* Verify that we don't use the alignment of struct S for inner accesses.  */

struct S
{
  double for_alignment;
  struct { int x, y, z; } a[16];
};

void f(struct S *s);

void f(struct S *s)
{
  unsigned int i;

  for (i = 0; i < 16; ++i)
    {
      s->a[i].x = 0;
      s->a[i].y = 0;
      s->a[i].z = 0;
    }
}

void
testTortureExecute (void)
{
#ifndef __SDCC_pdk14 // Lack of memory
  struct S s;
  f (&s);
  return;
#endif

}
