/*
  bug-2124.c
*/

#include <testfwk.h>

typedef int (*DT)(int);

unsigned int f1(unsigned int c)
{
  return c + 1;
}

int f2(int c)
{
  return c + 2;
}

DT gpfunc[] = {(DT) f1, f2};

void testBug (void)
{
  DT lpfunc[] = {(DT) f1, f2};
  DT spfunc[] = {f2, (DT) f1};
  ASSERT (gpfunc[0](0x55) == 0x56);
  ASSERT (gpfunc[1](0x66) == 0x68);
  ASSERT (lpfunc[0](0x77) == 0x78);
  ASSERT (lpfunc[1](0x44) == 0x46);
  ASSERT (spfunc[0](0x11) == 0x13);
  ASSERT (spfunc[1](0x22) == 0x23);
}
