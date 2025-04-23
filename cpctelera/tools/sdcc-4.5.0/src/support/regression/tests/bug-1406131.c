/* bug-1406131.c

   always false while-loop
*/
#include <testfwk.h>

void
testwhile(void)
{
  do
    ;
  while (0);
  
  while (0)
    ;
  
  for (; 0; )
    ;
}
