/*
        bugs 1292721.
*/

#include <testfwk.h>

char bar(void)
{
  static char ret = 0;
  if(!ret) {
    ret = 1;
    return(0);
  }
  return(1);
}

void
testBug156270(void)
{
  char aa, bb;

  aa = bar();

  for (;;) {

    bb = bar();

    if (!bb)
      break;
    
    if (aa == 0)
      return;

    ASSERT(0);
  }
  ASSERT(0);
}

