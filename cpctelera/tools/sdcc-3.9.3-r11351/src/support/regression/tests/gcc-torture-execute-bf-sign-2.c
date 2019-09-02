/*
bf-sign-2.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

/* 
 This test checks promotion of bitfields.  Bitfields should be promoted
 very much like chars and shorts: 

 Bitfields (signed or unsigned) should be promoted to signed int if their
 value will fit in a signed int, otherwise to an unsigned int if their 
 value will fit in an unsigned int, otherwise we don't promote them (ANSI/ISO
 does not specify the behavior of bitfields larger than an unsigned int).

 We test the behavior by subtracting two from the promoted value: this will
 result in a negitive value for signed types, a positive value for unsigned
 types.  This test (of course) assumes that the compiler is correctly 
 implementing signed and unsigned arithmetic.
 */

struct X {
  unsigned int	     u3:3;
#if 0 // Enable when SDCC supports bit-fields wider than 16 bits
    signed long int  s31:31;
    signed long int  s32:32;
  unsigned long int  u31:31;
  unsigned long int  u32:32;
  unsigned long long ull3 :3;
  unsigned long long ull35:35;
#endif
  unsigned u15:15;
};

struct X x;

void
testTortureExecute (void)
{
  if ((x.u3 - 2) >= 0)		/* promoted value should be signed */
    ASSERT (0);
#if 0
  if ((x.s31 - 2) >= 0)		/* promoted value should be signed */
    ASSERT (0);

  if ((x.s32 - 2) >= 0)		/* promoted value should be signed */
    ASSERT (0);
#endif
  if ((x.u15 - 2) >= 0)		/* promoted value should be signed */
    ASSERT (0);
#if 0
  /* Conditionalize check on whether integers are 4 bytes or larger, i.e.
     larger than a 31 bit bitfield.  */
  if (sizeof (int) >= 4)
    {
      if ((x.u31 - 2) >= 0)	/* promoted value should be signed */
	ASSERT (0);
    }
  else
    {
      if ((x.u31 - 2) < 0)	/* promoted value should be UNsigned */
	ASSERT (0);
    }

  if ((x.u32 - 2) < 0)		/* promoted value should be UNsigned */
    ASSERT (0);

  if ((x.ull3 - 2) >= 0)	/* promoted value should be signed */
    ASSERT (0);

  if ((x.ull35 - 2) < 0)	/* promoted value should be UNsigned */
    ASSERT (0);
#endif
  return;
}
