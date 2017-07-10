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

  ASSERT (atol ("-2147483648") == -2147483648l);
  ASSERT (atol ("2147483647") == 2147483647l);

#ifdef __SDCC_LONGLONG
  ASSERT (atoll ("-2147483648") == -2147483648l);
  ASSERT (atoll ("2147483647") == 2147483647l);
#endif
}

