/*
   950607-2.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

typedef struct {
  long int p_x, p_y;
} Point;

#if !(defined(__SDCC_mcs51) && defined(__SDCC_MODEL_SMALL)) && !defined(__SDCC_pdk14) &&!defined(__SDCC_pdk15) // Lack of memory
int
f (Point basePt, Point pt1, Point pt2)
{
  long long vector;

  vector =
    (long long) (pt1.p_x - basePt.p_x) * (long long) (pt2.p_y - basePt.p_y) -
      (long long) (pt1.p_y - basePt.p_y) * (long long) (pt2.p_x - basePt.p_x);

  if (vector > (long long) 0)
    return 0;
  else if (vector < (long long) 0)
    return 1;
  else
    return 2;
}
#endif

void
testTortureExecute (void)
{
#if !(defined(__SDCC_mcs51) && defined(__SDCC_MODEL_SMALL)) && !defined(__SDCC_pdk14) &&!defined(__SDCC_pdk15) // Lack of memory
  Point b, p1, p2;
  int answer;

  b.p_x = -23250;
  b.p_y = 23250;

  p1.p_x = 23250;
  p1.p_y = -23250;

  p2.p_x = -23250;
  p2.p_y = -23250;

  answer = f (b, p1, p2);

  if (answer != 1)
    ASSERT (0);
  return;
#endif
}

