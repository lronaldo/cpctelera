/*
   restrict-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Origin: Kaveh Ghazi <ghazi@caip.rutgers.edu> 2002-05-27.  */

extern int i;

extern int func0 (int);
extern int func1 (int);

/* GCC should automatically detect attributes for these functions.
   At -O3 They'll be inlined, but that's ok.  */
static int func2 (int a) { return i + a; } /* pure */
static int func3 (int a) { return a * 3; } /* const */
static int func4 (int a) { return func0(a) + a; } /* pure */
static int func5 (int a) { return a + func1(a); } /* const */
static int func6 (int a) { return func2(a) + a; } /* pure */
static int func7 (int a) { return a + func3(a); } /* const */

void
testTortureExecute (void)
{
  int i[10], r;

  i[0] = 0;
  r = func0(0);
  if (i[0])
    ASSERT (0);

  i[1] = 0;
  r = func1(0);
  if (i[1])
    ASSERT (0);

  i[2] = 0;
  r = func2(0);
  if (i[2])
    ASSERT (0);

  i[3] = 0;
  r = func3(0);
  if (i[3])
    ASSERT (0);

  i[4] = 0;
  r = func4(0);
  if (i[4])
    ASSERT (0);

  i[5] = 0;
  r = func5(0);
  if (i[5])
    ASSERT (0);

  i[6] = 0;
  r = func6(0);
  if (i[6])
    ASSERT (0);

  i[7] = 0;
  r = func7(0);
  if (i[7])
    ASSERT (0);

  return;
}

int func0 (int a) { return a - i; } /* pure */
int func1 (int a) { return a - a; } /* const */

int i = 2;

