/** Compare with 2^n greater then 255
    test for patch #2702889 - Summary of all uncommitted changes I applied on "my" SDCC
*/

#include <testfwk.h>

foreach(`int_right', (0x0080, 0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000), `
int
lwr_if_`'int_right (unsigned left)
{ 
  if (left < int_right)
    return 1;
  else
    return 0;
}

int
lwr_`'int_right (unsigned left)
{
  return left < int_right;
}

void
test_lwr_`'int_right (void)
{
  ASSERT (lwr_if_`'int_right (int_right - 1));
  ASSERT (!lwr_if_`'int_right (int_right));
  ASSERT (!lwr_if_`'int_right (int_right + 1));

  ASSERT (lwr_`'int_right (int_right - 1));
  ASSERT (!lwr_`'int_right (int_right));
  ASSERT (!lwr_`'int_right (int_right + 1));
}
')dnl

foreach(`int_right', (0x007f, 0x00ff, 0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff), `
int
gtr_if_`'int_right (unsigned left)
{
  return (left > int_right) ? 1 : 0;
}

int
gtr_`'int_right (unsigned left)
{
  return left > int_right;
}

void
test_gtr_`'int_right (void)
{
  ASSERT (gtr_if_`'int_right (int_right + 1));
  ASSERT (!gtr_if_`'int_right (int_right));
  ASSERT (!gtr_if_`'int_right (int_right - 1));

  ASSERT (gtr_`'int_right (int_right + 1));
  ASSERT (!gtr_`'int_right (int_right));
  ASSERT (!gtr_`'int_right (int_right - 1));
}
')dnl

/* special case for 0x0000 */
lwr_if_0x0000 (unsigned left)
{
  if (left < 0x0000)
    return 1;
  else
    return 0;
}

int
lwr_0x0000 (unsigned left)
{
  return left < 0x0000;
}

void
test_lwr_0x0000 (void)
{
  ASSERT (!lwr_if_0x0000 (0x0000 - 1));
  ASSERT (!lwr_if_0x0000 (0x0000));
  ASSERT (!lwr_if_0x0000 (0x0000 + 1));

  ASSERT (!lwr_0x0000 (0x0000 - 1));
  ASSERT (!lwr_0x0000 (0x0000));
  ASSERT (!lwr_0x0000 (0x0000 + 1));
}

#if !defined(PORT_HOST)
/* special case for 0xffff */
int
gtr_if_0xffff (unsigned left)
{
  return (left > 0xffff) ? 1 : 0;
}

int
gtr_0xffff (unsigned left)
{
  return left > 0xffff;
}
#endif

void
test_gtr_0xffff (void)
{
#if !defined(PORT_HOST)
  ASSERT (!gtr_if_0xffff (0xffff + 1));
  ASSERT (!gtr_if_0xffff (0xffff));
  ASSERT (!gtr_if_0xffff (0xffff - 1));

  ASSERT (!gtr_0xffff (0xffff + 1));
  ASSERT (!gtr_0xffff (0xffff));
  ASSERT (!gtr_0xffff (0xffff - 1));
#endif
}

/* unsigned long test cases */
#define LONG_RIGHT_LWR 0x80000000
int
long_lwr (unsigned long left)
{
  return (left < LONG_RIGHT_LWR) ? 1 : 0;
}

#define LONG_RIGHT_GTR 0x7fffffff
int
long_gtr (unsigned long left)
{
  if (left > LONG_RIGHT_GTR)
    return 1;
  else
    return 0;
}

void
test_lwr_gtr (void)
{
  ASSERT (long_lwr (LONG_RIGHT_LWR - 1));
  ASSERT (!long_lwr (LONG_RIGHT_LWR));
  ASSERT (!long_lwr (LONG_RIGHT_LWR + 1));

  ASSERT (long_gtr (LONG_RIGHT_GTR + 1));
  ASSERT (!long_gtr (LONG_RIGHT_GTR));
  ASSERT (!long_gtr (LONG_RIGHT_GTR - 1));
}

/* Test for bug #2165 Broken comparison */
char c;

int
bug_2165 (unsigned i)
{ 
  if (i < 0x0100)
    return 1;
  else
    {c = i < 0x0100; return 0;}
}

void
test_bug_bug_2165 (void)
{
  ASSERT (bug_2165 (254));
  ASSERT (bug_2165 (255));
  ASSERT (!bug_2165 (256));
}
