/** abs.c
*/
#include <testfwk.h>
#include <stdlib.h>

void
testAbs(void)
{
  ASSERT( abs(0x7fff) == 0x7fff );
  ASSERT( abs(-1000)  == 1000 );
  ASSERT( abs(-32767) == 32767 );

  ASSERT( labs(0x7FFFffffuL) == 0x7FFFffffuL );
  ASSERT( labs(-1000000L)    == 1000000L );
  ASSERT( labs(-2147483647L) == 2147483647L );
}

