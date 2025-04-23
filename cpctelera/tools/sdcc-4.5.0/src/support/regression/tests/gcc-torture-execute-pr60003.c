/*
   pr60003.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if !defined(__SDCC_pic14) // Unimplemented setjmp
#include <setjmp.h>

/* PR tree-optimization/60003 */
/* { dg-require-effective-target indirect_jumps } */

jmp_buf buf;

void
baz (void)
{
  longjmp (buf, 1);
}

void
bar (void)
{
  baz ();
}

int
foo (int x)
{
  volatile int a = 0;

  if (setjmp (buf) == 0)
    {
      while (1)
	{
	  a = 1;
	  bar ();  /* OK if baz () instead */
	}
    }
  else
    {
      if (a == 0)
	return 0;
      else
	return x;
    }
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_pic14) // Unimplemented setjmp
  if (foo (1) == 0)
    ASSERT (0);
#endif
}

