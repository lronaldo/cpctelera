/*
   bug1409955.c
*/

#include <testfwk.h>

void
WriteToXData (char *buffer)
{
  (void)buffer;
}

void
test_push_pop (void)
{
  char a;
  __xdata char *p;
  char d[5];

  d[0] = 0;
  d[1] = 0;
  d[2] = 0;
  d[3] = 0;
  d[4] = 0;
  p = 0;
  do
    {
      if ((unsigned short)p > 10)
        a = 10 - (char)p;
      else
        a = 60;
      d[0] = 0x12;
      d[1] = 0x34;
      WriteToXData (d); // Watch the xdata: 0, 1!
      p += a;
      ASSERT (d[0] == 0x12);
      ASSERT (d[1] == 0x34);
    }
  while (!p);
  d[0] = 1;
  d[1] = 2;
  d[2] = 3;
  d[3] = 4;
  d[4] = 5;
}
