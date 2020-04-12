/*
   bug1057979.c
*/

#include <testfwk.h>

int srlen;

int l_regrd(int ad)
{
  return ad;
}

void head_send(int i)
{
  srlen= i;
  if ((l_regrd(0x1234) & 0x0200) == 0)
  {
    srlen= 0;
  }
}

void
test_1185672(void)
{
  head_send(1);
  ASSERT( srlen == 1 );
}
