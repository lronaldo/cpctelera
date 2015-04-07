/*
   950628-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when sdcc supports struct!
#if 0
typedef struct
{
  char hours, day, month;
  short year;
} T;

T g (void)
{
  T now;

  now.hours = 1;
  now.day = 2;
  now.month = 3;
  now.year = 4;
  return now;
}

T f (void)
{
  T virk;

  virk = g ();
  return virk;
}
#endif

void
testTortureExecute (void)
{
#if 0
  if (f ().hours != 1 || f ().day != 2 || f ().month != 3 || f ().year != 4)
    ASSERT (0);
  return;
#endif
}

