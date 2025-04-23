/* assert.c
 */

#include <testfwk.h>

#define NDEBUG

#include <assert.h>

volatile int i = 42;

void testStaticAssert (void)
{
/* sdcc always supports C11 _Static_assert, even though the earliest standard requiring it is C11. */
#if defined (__SDCC) || __STDC_VERSION__ >= 201112L
  _Static_assert (1, "First assertion");
  _Static_assert (sizeof(int), "Second assertion");
#endif
}

#pragma std_c23

void testStaticAssert2X (void)
{
#if defined (__SDCC)
  _Static_assert (1);
  _Static_assert (sizeof(int));
#endif
}

int a[] = {1, 0};

void testAssert (void)
{
  assert (i);
  assert (i - 42);
#ifdef __SDCC // SDCC assert is always C23-compliant
  assert (a[1, 0]); // C23 requires C23 to be implemented as variadic macro, which is meant for the use of compound literals in the argument. But the easiest way to test that is by using a comma operator not surrounded by ().
#endif
}

