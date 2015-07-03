/** Simple set of tests for floating pt.
 */
#include <testfwk.h>
#include <math.h>

#if (PORT_HOST)
#  define FCOMP(a,b) (fabsf((a) - (b)) < ((b) * 1e-7))
#else
   /* Testing floats for equality is normally a bug,
      but too keep this test simple we dare it. And
      it works with the exception of the division on
      the host port. */
#  define FCOMP(a,b) ((a) == (b))
#endif

void
testCmp (void)
{
  volatile float left, right;

  left = 5;
  right = 13;
  ASSERT (left + right == 18);
  ASSERT (left + right <= 18);
  ASSERT (left + right >= 18);
  ASSERT (left + right > 17.9);
  ASSERT (left + right < 18.1);
}

void
testDiv (void)
{
#if defined (__SDCC_mcs51) && !defined (__SDCC_STACK_AUTO)
  __idata __at 0xd0
#endif
  volatile float left;
  volatile float right;

  left = 17;
  right = 343;

  ASSERT (FCOMP (left / right, (17.0 / 343.0)));
  ASSERT (FCOMP (right / left, (343.0 / 17.0)));

  right = 17;
  ASSERT (FCOMP (left / right, 1.0));
}

void
testDivNearOne (void)
{
  volatile float left, right, result;

  left = 12392.4;
  right = 12392.4;
  result = left / right;

  if (result > 0.999999)
    {
      /* Fine */
    }
  else
    {
      FAIL ();
    }
  if (result < 1.00001)
    {
      /* Fine */
    }
  else
    {
      FAIL ();
    }
  if (result > 0.999999 && result < 1.00001)
    {
      /* Fine */
    }
  else
    {
      FAIL ();
    }
}
