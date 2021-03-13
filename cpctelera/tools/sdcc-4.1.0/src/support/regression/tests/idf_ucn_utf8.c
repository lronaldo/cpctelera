/** Test UCNs and utf8 in identifiers
*/
#include <testfwk.h>

#if !defined(PORT_HOST) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
const unsigned long µ_div = 1000;
#endif

void
testShadowing(void)
{
#if !defined(PORT_HOST) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
  const unsigned long \u00b5_div = 1000000uL;
  ASSERT(µ_div == 1000000uL);
  ASSERT(\u00B5_div == 1000000uL);
#endif
}

void
testAssignment(void)
{
#if !defined(PORT_HOST) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
  int \u00F6 = 0;
  \u00f6 = 1;
  ö = 2;
  ASSERT(\U000000F6 == 2);
#endif
}

void
testSquared(void)
{
#if !defined(PORT_HOST) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
  const float e = 2.71828182846;
  const float e² = e * e;
  const float e_squared = e * e;
  ASSERT(e\u00B2 == e_squared);
#endif
}
