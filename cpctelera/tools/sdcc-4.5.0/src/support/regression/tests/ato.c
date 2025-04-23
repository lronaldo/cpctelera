/** ato.c
*/
#include <testfwk.h>
#include <stdlib.h>

void
testAto(void)
{
  ASSERT (atoi ("23") == 23);
  ASSERT (atoi ("023") == 23);
  ASSERT (atoi ("+23") == +23);
  ASSERT (atoi ("-23") == -23);
  ASSERT (atoi ("-32768") == -32768);
  ASSERT (atoi ("+32767") == +32767);

#ifndef __SDCC_pdk14 // Not enough RAM
#if !(defined (__SDCC_pdk15) && defined(__SDCC_STACK_AUTO)) // Lack of code memory
  ASSERT (atol ("-2147483648") == -2147483648l);
  ASSERT (atol ("2147483647") == 2147483647l);

#ifndef __SDCC_pdk15 // Not enough RAM
#ifdef __SDCC_LONGLONG
  ASSERT (atoll ("-2147483648") == -2147483648l);
  ASSERT (atoll ("2147483647") == 2147483647l);
#endif
#endif
#endif
#endif
}

