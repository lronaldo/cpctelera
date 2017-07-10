/* bug 1505811
 *   demonstrates an incorrect "loopreverse"
 *   note func0, is a kind of safeguard, the incorrect code
 *     will access indices 0 and 1 instead of 1 and 2,
 *     and with incorrect order
 */

#include <testfwk.h>

char glbl;

void func0() { glbl = 0; }
void func1() { glbl = 1; }
void func2() { glbl = 2; }

typedef void (*fptr)();

fptr ep_init[3] = { func0, func1, func2 };

void buggy()
{
  unsigned char i;
  for(i = 1; i <= 2; i++)
  {
    ep_init[i]();
  }
}

void
testBug(void)
{
  buggy();
  ASSERT(glbl == 2);
}
