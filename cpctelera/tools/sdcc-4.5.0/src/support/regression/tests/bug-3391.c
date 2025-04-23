/*
   bug-3166.c
   Due to a check in SDCCval.c using double instead of long long (thus losing precision, 0x8000000000000000 was given the type of signed long long instead of unsigned long long.
 */

#include <testfwk.h>

#include <limits.h>

#define IS_ULL(x) _Generic((x), unsigned long long: 1, default: 0)
#define IS_UL(x) _Generic((x), unsigned long: 1, default: 0)

void testBug(void)
{
#if (LLONG_MAX <= 0x7fffffffffffffffLL)
  ASSERT( IS_ULL(0x8000000000000000ll)); // Bug triggered here.
  ASSERT(!IS_ULL(0x7fffffffffffffffll));
#endif
  ASSERT(!IS_ULL(0x0000000080000000ll));

#if (LONG_MAX <= 0x7fffffffL)
  ASSERT( IS_UL(0x80000000l));
  ASSERT(!IS_UL(0x7fffffffl));
#endif
  ASSERT(!IS_UL(0x00008000l));
}

