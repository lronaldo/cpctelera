/** not.c test ! operator

  ANSI: return type is int

  attr: volatile,
*/
#include <testfwk.h>

void
testNot(void)
{
    signed char  {attr} sc;
  unsigned char  {attr} uc;
  unsigned int   {attr} ui;
  unsigned long  {attr} ul;

  sc = 0;
  uc = 0;
  ui = 0;
  ul = 0;
  /* remember: unsigned * signed -> unsigned */
  /*             signed * signed ->   signed */
  ASSERT(!(  signed char) 0 * -1 < 0);
  ASSERT(!(unsigned char) 0 * -1 < 0);
  ASSERT(!sc   * -1 < 0);
  ASSERT(!uc   * -1 < 0);
  ASSERT(! 0   * -1 < 0);
  ASSERT(! 0u  * -1 < 0);
  ASSERT(!ui   * -1 < 0);
  ASSERT(! 0l  * -1 < 0);
  ASSERT(! 0ul * -1 < 0);
  ASSERT(!ul   * -1 < 0);

  ASSERT(!(char) 0 <<  8 == 0x100);
  ASSERT(!sc       <<  8 == 0x100);
#if !defined(PORT_HOST)
  ASSERT(!0  << 16 == 0);
  ASSERT(!0l << 16 == 0);
  ASSERT(!ui << 16 == 0);
  ASSERT(!ul << 16 == 0);
#endif
}
