/*
   961206-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when sdcc supports long long in these ports!
#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
int
sub1 (unsigned long long i)
{
  if (i < 0x80000000)
    return 1;
  else
    return 0;
}

int
sub2 (unsigned long long i)
{
  if (i <= 0x7FFFFFFF)
    return 1;
  else
    return 0;
}

int
sub3 (unsigned long long i)
{
  if (i >= 0x80000000)
    return 0;
  else
    return 1;
}

int
sub4 (unsigned long long i)
{
  if (i > 0x7FFFFFFF)
    return 0;
  else
    return 1;
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
  if (sub1 (0x80000000ULL))
    ASSERT (0);

  if (sub2 (0x80000000ULL))
    ASSERT (0);

  if (sub3 (0x80000000ULL))
    ASSERT (0);

  if (sub4 (0x80000000ULL))
    ASSERT (0);

  return;
#endif
}

