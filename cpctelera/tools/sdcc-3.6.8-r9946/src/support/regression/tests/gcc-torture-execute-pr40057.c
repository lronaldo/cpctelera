/*
   pr40057.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when sdcc supports long long in these ports!
#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
/* PR middle-end/40057 */

int
foo (unsigned long long x)
{
  unsigned long long y = (x >> 31ULL) & 1ULL;
  if (y == 0ULL)
    return 0;
  return -1;
}

int
bar (long long x)
{
  long long y = (x >> 31LL) & 1LL;
  if (y == 0LL)
    return 0;
  return -1;
}
#endif

void
testTortureExecute (void)
{
#ifndef PORT_HOST // Fails on NetBSD
#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
  if (sizeof (long long) != 8)
    return;
  if (foo (0x1682a9aaaULL))
    ASSERT (0);
  if (!foo (0x1882a9aaaULL))
    ASSERT (0);
  if (bar (0x1682a9aaaLL))
    ASSERT (0);
  if (!bar (0x1882a9aaaLL))
    ASSERT (0);
  return;
#endif
#endif
}

