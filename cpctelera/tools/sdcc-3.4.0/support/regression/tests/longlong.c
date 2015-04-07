/** Simple long long tests.

 */
#include <testfwk.h>

#ifdef __SDCC
#pragma std_sdcc99
#pragma disable_warning 85
#endif

#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
long long x;
unsigned long long y;
int i;

long long g(void)
{
  int y = i + 1;
  return (y);
}

long long h(void)
{
  return (x);
}

long long c(void)
{
  return (12ll);
}

long long d(int i)
{
  return (i);
}

long long (*gp)(void) = &g;
#endif

void
testLongLong (void)
{
  volatile unsigned long tmp;
// Test fils on 32-bit systems.
//#if !defined(__SDCC_mcs51) && !defined(__SDCC_hc08) && !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
#if 0
  i = 42;
  ASSERT (g() == 43);
  i = 23;
  ASSERT ((*gp)() == 24);
  ASSERT (c() == 12);
  x = 42;
  ASSERT (h() == x);
  ASSERT (d(12) == 12ll);
  ASSERT ((x >> 1) == 21);
  ASSERT ((x << 1) == 84);
  ASSERT (!(x >> 17));
//  ASSERT ((x << 17) == (42l << 17)); sdcc has broken long long constants!
  y = x;
  ASSERT (y == 42ull);
  ASSERT ((y >> 1) == 21);
  ASSERT ((y << 1) == 84);
  ASSERT ((y >> 17) == 0);
//  ASSERT ((y << 17) == (42ul << 17));
//  ASSERT ((y << 16) == (42ul << 16));

  tmp = 0xaaffaafful;
  y = tmp;
  ASSERT (c() == 12);
  ASSERT ((unsigned char)y == (unsigned char)tmp);
  ASSERT ((unsigned int)y == (unsigned int)tmp);
  ASSERT ((unsigned long)y == (unsigned long)tmp);
  ASSERT (y == tmp);
  ASSERT ((y >> 8) == (tmp >>= 8));
  ASSERT ((y >> 12) == (tmp >>= 4));
  ASSERT ((y >> 16) == (tmp >>= 4));
  ASSERT ((y >> 24) == (tmp >>= 8));
  ASSERT ((y >> 32) == 0x0ul);

  tmp = 1;
  y = tmp;
  ASSERT (c() == 12);
  ASSERT ((y << 1) == 2);
  ASSERT ((y << 16) == (tmp << 16));
  ASSERT ((y << 23) == (tmp << 23));
  y += 2;
  y <<= 31;
  ASSERT (c() == 12);
  y >>= 31;
  ASSERT (y == 3);
#endif
}

