/* assert.c
 */

#include <testfwk.h>

#define NDEBUG

#include <assert.h>

volatile int i = 42;

void testStaticAssert (void)
{
/* sdcc always supports _Static_assert, even though the earliest standard requiring it is C11. */
#if defined (__SDCC) || __STDC_VERSION__ >= 201112L
  _Static_assert (1, "First assertion");
  _Static_assert (sizeof(int), "Second assertion");
#endif
}

void testAssert (void)
{
  assert (i);
  assert (i - 42);
}

