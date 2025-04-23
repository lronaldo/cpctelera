/** Simple long long tests.
 test: mul, div, bit, shift

 */
#include <testfwk.h>

#ifdef __SDCC
#pragma std_sdcc99
#endif

#define TEST_{test}

#if !(defined(__SDCC_mcs51) && !defined(__SDCC_STACK_AUTO) && defined(__SDCC_MODEL_SMALL) ) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
long long x;
unsigned long long y;
int i;

#if defined(TEST_mul)

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

void
LongLong_mul (void)
{
  volatile unsigned long tmp;

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
  ASSERT (y * tmp == 42 * 23);
  ASSERT (y / tmp == 42 / 23);
  ASSERT (y % tmp == 42 % 23);

  tmp = 42;
  x = 42ll << 23;
  ASSERT (x + y == (42ll << 23) + 42);
  ASSERT (x - y == (42ll << 23) - 42);
  ASSERT (x * y == (42ll << 23) * 42);
  ASSERT (x / tmp == (42ll << 23) / 42);
  ASSERT (x % tmp == (42ll << 23) % 42);

  x = 0x1122334455667788ll;
  y = 0x9988776655443322ull;
  ASSERT (y + x == 0x9988776655443322ull + 0x1122334455667788ll);
  ASSERT (y - x == 0x9988776655443322ull - 0x1122334455667788ll);

  ASSERT (mulLL (1ull, 1ull) == 1ull * 1ull);

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

  c(); // Unused long long return value requires special handling in register allocation.
}

#elif defined(TEST_div)

static unsigned long long divULL(unsigned long long a, unsigned long long b)
{
  return a / b;
}

static unsigned long long modULL(unsigned long long a, unsigned long long b)
{
  return a % b;
}

static long long divLL(long long a, long long b)
{
  return a / b;
}

static long long modLL(long long a, long long b)
{
  return a % b;
}

void
LongLong_div (void)
{
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
}

#elif defined(TEST_bit)

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

void
LongLong_bit (void)
{
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
}

#elif defined(TEST_shift)

void
LongLong_shift (void)
{
  unsigned char i,j,expected,match;
  for (i=0;i<64;i++) {
    y = 1ull << i;
    match=0;
    for (j=0;j<64;j++) {
      expected = (j==i);
      if ((unsigned char)(y & 1) == expected)
        match++;
      y >>= 1;
    }
    ASSERT (match==64);
  }

  for (i=0;i<64;i++) {
    y = 0x8000000000000000ull >> i;
    match=0;
    for (j=0;j<64;j++) {
      expected = (j==i);
      if ((y & 0x8000000000000000ull) ? expected : !expected)
        match++;
      y <<= 1;
    }
    ASSERT (match==64);
  }

  for (i=0;i<64;i++) {
    x = (signed long long)0x8000000000000000ll >> i;
    match=0;
    for (j=0;j<64;j++) {
      expected = (j<=i);
      if ((x & 0x8000000000000000ll) ? expected : !expected)
        match++;
      x <<= 1;
    }
    ASSERT (match==64);
  }
 
}

#endif //TEST_mul/div/bit/shift

#endif //!mcs51-small

void
testLongLong (void)
{
#if !(defined(__SDCC_mcs51) && !defined(__SDCC_STACK_AUTO) && defined(__SDCC_MODEL_SMALL) ) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
  LongLong_{test}();
#endif
}

