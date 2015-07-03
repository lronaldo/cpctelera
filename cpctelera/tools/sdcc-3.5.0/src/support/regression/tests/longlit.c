/* Testing arithmetics with long litterals
 */

#include <testfwk.h>

#define OSCILLATOR 11059200
#define BAUD 19200L

#define T1_RELOAD_VALUE -(2*OSCILLATOR)/(32*12*BAUD)

static unsigned char T1=T1_RELOAD_VALUE;

void
testLongLit(void)
{
  ASSERT(T1==0xfd);
}
