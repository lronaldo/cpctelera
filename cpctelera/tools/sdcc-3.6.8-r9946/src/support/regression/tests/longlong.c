/** Simple long long tests.

 */
#include <testfwk.h>

#ifdef __SDCC
#pragma std_sdcc99
#pragma disable_warning 85
#pragma disable_warning 212
#endif

#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_ds400) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
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

static unsigned long long mulLL(unsigned long long a, unsigned long long b)
{
  return a * b;
}

static long long divLL(long long a, long long b)
{
  return a / b;
}

static unsigned long long divULL(unsigned long long a, unsigned long long b)
{
  return a / b;
}

static unsigned long long modULL(unsigned long long a, unsigned long long b)
{
  return a % b;
}

static long long modLL(long long a, long long b)
{
  return a % b;
}

static int compareLL(unsigned long long a, unsigned long long b)
{
  if (a > b)
    return 1;
  else if (a < b)
    return -1;
  else
    return 0;
}

static long long leftShiftLL(long long a)
{
  return a << 8;
}
 
static long long rightShiftLL(long long a)
{
  return a >> 8;
}

static unsigned long long rightShiftULL(unsigned long long a)
{
  return a >> 8;
}

static unsigned long long leftShiftULL(unsigned long long a)
{
  return a << 8;
}

static unsigned long long bitAndULL(unsigned long long a, unsigned long long b)
{
  return a & b;
}

static unsigned long long bitOrULL(unsigned long long a, unsigned long long b)
{
  return a | b;
}

static unsigned long long bitXorULL(unsigned long long a, unsigned long long b)
{
  return a ^ b;
}

static unsigned long long bitNotULL(unsigned long long a)
{
  return ~a;
}

#endif

void
testLongLong (void)
{
  volatile unsigned long tmp;

#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_ds400) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_hc08) && !defined(__SDCC_s08)
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
  ASSERT ((x << 17) == (42l << 17));
  y = x;
  ASSERT (y == 42ull);
  ASSERT ((y >> 1) == 21);
  ASSERT ((y << 1) == 84);
  ASSERT ((y >> 17) == 0);
  ASSERT ((y << 17) == (42ul << 17));
  ASSERT ((y << 16) == (42ul << 16));

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

  tmp = 23;
  y = 42;
  ASSERT (y + tmp == 42 + 23);
  ASSERT (y - tmp == 42 - 23);
#ifndef __SDCC_gbz80 // long long multiplication broken on gbz80, bug #2329
  ASSERT (y * tmp == 42 * 23);
#endif
  ASSERT (y / tmp == 42 / 23);
  ASSERT (y % tmp == 42 % 23);

  tmp = 42;
  x = 42ll << 23;
  ASSERT (x + y == (42ll << 23) + 42);
#ifndef __SDCC_gbz80 // Breaks due to bug in hl handling in gbz80 port
  ASSERT (x - y == (42ll << 23) - 42);
#endif
#ifndef __SDCC_gbz80 // long long multiplication broken on gbz80, bug #2329
  ASSERT (x * y == (42ll << 23) * 42);
#endif
  ASSERT (x / tmp == (42ll << 23) / 42);
  ASSERT (x % tmp == (42ll << 23) % 42);

  x = 0x1122334455667788ll;
  y = 0x9988776655443322ull;
  ASSERT (y + x == 0x9988776655443322ull + 0x1122334455667788ll);
  ASSERT (y - x == 0x9988776655443322ull - 0x1122334455667788ll);

#ifndef __SDCC_gbz80 // long long multiplication broken on gbz80, bug #2329
  y = 0x55667788ull;
  ASSERT (y * y == 0x55667788ull * 0x55667788ull); // this test is optimized by constant propagation
  ASSERT (mulLL (y, y) == 0x55667788ull * 0x55667788ull); // this test is not
  y = 0x55667788ull;
  x = 0x55667788ll;
  ASSERT (y * x == 0x55667788ull * 0x55667788ll); // this test is optimized by constant propagation
  ASSERT (mulLL (y, x) == 0x55667788ull * 0x55667788ll); // this test is not

  y = 0xa5667788ull;
  ASSERT (y * y == 0xa5667788ull * 0xa5667788ull); // this test is optimized by constant propagation
  ASSERT (mulLL (y, y) == 0xa5667788ull * 0xa5667788ull); // this test is not
  y = 0xa5667788ull;
  x = 0xa5667788ll;
  ASSERT (y * x == 0xa5667788ull * 0xa5667788ll); // this test is optimized by constant propagation
  ASSERT (mulLL (y, x) == 0xa5667788ull * 0xa5667788ull); // this test is not

  y = 0xa5667788ccddull;
  x = 0x0788ll;
  ASSERT (y * x == 0xa5667788ccddull * 0x0788ll); // this test is optimized by constant propagation
  ASSERT (mulLL (y, x) == 0xa5667788ccddull * 0x0788ll); // this test is not

  y = 0x1122334455667700ull;
  x = 0x2ll;
  ASSERT (y * x == 0x1122334455667700ull * 0x2ll); // this test is optimized by constant propagation
  ASSERT (mulLL (y, x) == 0x1122334455667700ull * 0x2ll); // this test is not
#endif

  y = 0x1122334455667700ull;
  x = 0x7ll;
  ASSERT (y / x == 0x1122334455667700ull / 0x7ll); // this test is optimized by constant propagation
  ASSERT (divULL (y, x) == 0x1122334455667700ull / 0x7ll); // this test is not
  ASSERT (y % x == 0x1122334455667700ull % 0x7ll); // this test is optimized by constant propagation
  ASSERT (modULL (y, x) == 0x1122334455667700ull % 0x7ll); // this test is not
  x = 0x1122334455667700ll;
  ASSERT (x / 0x7ll == 0x1122334455667700ll / 0x7ll); // this test is optimized by constant propagation
  ASSERT (divLL (x, 0x7ll) == 0x1122334455667700ll / 0x7ll); // this test is not
  ASSERT (x % 0x7ll == 0x1122334455667700ll % 0x7ll); // this test is optimized by constant propagation
  ASSERT (modLL (x, 0x7ll) == 0x1122334455667700ll % 0x7ll); // this test is not

  y = 0x44556677aabbccddull;
  x = 0x7766554433221100ull;
  ASSERT (y < x);
  ASSERT (x > y);
  ASSERT (compareLL (y, x) == -1);
  ASSERT (compareLL (x, y) == 1);

  y = 0x5566778899aabbccull;
  x = 0xaabbccdd11223344ll;
  ASSERT ((y << 8) == 0x66778899aabbcc00ull);
  ASSERT (leftShiftULL (y) == 0x66778899aabbcc00ull);
  ASSERT ((y >> 8) == 0x005566778899aabbull);
  ASSERT (rightShiftULL (y) == 0x005566778899aabbull);
  ASSERT ((x << 8) == 0xbbccdd1122334400ll);
  ASSERT (leftShiftLL (x) == 0xbbccdd1122334400ll);
  ASSERT ((x >> 8) == 0xffaabbccdd112233ll);
  ASSERT (rightShiftLL (x) == 0xffaabbccdd112233ll);

  x = 0x44556677aabbccddll;
  ASSERT (++x == 0x44556677aabbccdell);
  y = 0x99556677aabbccddull;
  ASSERT (--y == 0x99556677aabbccdcull);

  y = 0x69aaaaaaaaaa55aaull;
  x = 0x69555555555555aall;
  ASSERT ((y & x) == (0x69aaaaaaaaaa55aaull & 0x69555555555555aall));
  ASSERT (bitAndULL (y, x) == (0x69aaaaaaaaaa55aaull & 0x69555555555555aall));
  ASSERT ((y | x) == (0x69aaaaaaaaaa55aaull | 0x69555555555555aall));
  ASSERT (bitOrULL (y, x) == (0x69aaaaaaaaaa55aaull | 0x69555555555555aall));
  ASSERT ((y ^ x) == (0x69aaaaaaaaaa55aaull ^ 0x69555555555555aall));
  ASSERT (bitXorULL (y, x) == (0x69aaaaaaaaaa55aaull ^ 0x69555555555555aall));
  ASSERT ((~y) == (~0x69aaaaaaaaaa55aaull));
  ASSERT (bitNotULL (y) == (~0x69aaaaaaaaaa55aaull));

  c(); // Unused long long return value require special handling in register allocation.
#endif
}

