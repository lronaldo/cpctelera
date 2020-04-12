/*
   bug-2885.c
   INT_MIN was not of type int.
 */

#include <testfwk.h>

#include <limits.h>

void testBug (void)
{
  ASSERT(sizeof(INT_MIN) == sizeof(int));
  ASSERT(sizeof(INT_MAX) == sizeof(int));
  ASSERT(sizeof(UINT_MAX) == sizeof(unsigned int));
  ASSERT(sizeof(LONG_MIN) == sizeof(long));
  ASSERT(sizeof(LONG_MAX) == sizeof(long));
  ASSERT(sizeof(ULONG_MAX) == sizeof(unsigned long));
  ASSERT(sizeof(LLONG_MIN) == sizeof(long long));
  ASSERT(sizeof(LLONG_MAX) == sizeof(long long));
  ASSERT(sizeof(ULLONG_MAX) == sizeof(unsigned long long));
}

