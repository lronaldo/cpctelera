/*
   20041011-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when these ports support long long!
#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)

typedef unsigned long long ull;
volatile int gvol[32];
ull gull;

#define MULTI(X) \
	X( 1), X( 2), X( 3), X( 4), X( 5), X( 6), X( 7), X( 8), X( 9), X(10), \
	X(11), X(12), X(13), X(14), X(15), X(16), X(17), X(18), X(19), X(20), \
	X(21), X(22), X(23), X(24), X(25), X(26), X(27), X(28), X(29), X(30)

#define DECLARE(INDEX) x##INDEX
#define COPYIN(INDEX) x##INDEX = gvol[INDEX]
#define COPYOUT(INDEX) gvol[INDEX] = x##INDEX

#define BUILD_TEST(NAME, N)		\
  ull		\
  NAME (int n, ull x)			\
  {					\
    while (n--)				\
      {					\
	int MULTI (DECLARE);		\
	MULTI (COPYIN);			\
	MULTI (COPYOUT);		\
	x += N;				\
      }					\
    return x;				\
  }

#define RUN_TEST(NAME, N)		\
  if (NAME (3, ~0ULL) != N * 3 - 1)	\
    ASSERT (0);				\
  if (NAME (3, 0xffffffffULL)		\
      != N * 3 + 0xffffffffULL)		\
    ASSERT (0);

#define DO_TESTS(DO_TEST)	\
  DO_TEST (t1, -2048)		\
  DO_TEST (t2, -513)		\
  DO_TEST (t3, -512)		\
  DO_TEST (t4, -511)		\
  DO_TEST (t5, -1)		\
  DO_TEST (t6, 1)		\
  DO_TEST (t7, 511)		\
  DO_TEST (t8, 512)		\
  DO_TEST (t9, 513)		\
  DO_TEST (t10, gull)		\
  DO_TEST (t11, -gull)

DO_TESTS (BUILD_TEST)

ull neg (ull x) { return -x; }
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
#if !defined(__SDCC_gbz80) // bug #2329
#if !defined(__SDCC_hc08) && !defined(__SDCC_s08)
  gull = 100;
  DO_TESTS (RUN_TEST)
  if (neg (gull) != -100ULL)
    ASSERT (0);
  return;
#endif
#endif
#endif
}
