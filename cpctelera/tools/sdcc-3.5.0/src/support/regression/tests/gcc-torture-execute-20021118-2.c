/*
   20021118-2.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Since these tests use function pointers with multiple parameters */
/* we can only proceed if we are compiling reentrant functions. */

#ifdef __SDCC_STACK_AUTO

/* Originally added to test SH constant pool layout.  t1() failed for
   non-PIC and t2() failed for PIC.  */

int t1 (float *f, int i,
	void (*f1) (double),
	void (*f2) (float, float))
{
  f1 (3.0);
  f[i] = f[i + 1];
  f2 (2.5f, 3.5f);
}

int t2 (float *f, int i,
	void (*f1) (double),
	void (*f2) (float, float),
	void (*f3) (float))
{
  f3 (6.0f);
  f1 (3.0);
  f[i] = f[i + 1];
  f2 (2.5f, 3.5f);
}

void f1 (double d)
{
  if (d != 3.0)
    ASSERT (0);
}

void f2 (float f1, float f2)
{
  if (f1 != 2.5f || f2 != 3.5f)
    ASSERT (0);
}

void f3 (float f)
{
  if (f != 6.0f)
    ASSERT (0);
}
#endif

void
testTortureExecute (void)
{
#if __SDCC_STACK_AUTO
  float f[3] = { 2.0f, 3.0f, 4.0f };
  t1 (f, 0, f1, f2);
  t2 (f, 1, f1, f2, f3);
  if (f[0] != 3.0f && f[1] != 4.0f)
    ASSERT (0);
  return;
#endif
}
