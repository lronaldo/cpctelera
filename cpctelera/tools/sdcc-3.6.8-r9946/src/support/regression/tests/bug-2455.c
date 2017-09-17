/*
   bug-2455.c
*/

#include <testfwk.h>

typedef int (*funcType) (int);

int foo0 (int a)
{
  return a + 2;
}

int foo1 (int a)
{
  return a - 6;
}

int foo2 (int a)
{
  return a * 2;
}

struct
{
  funcType fpa[2];
  funcType fpb;
} testS = {{foo0, foo1}, foo2};

void testBug (void)
{
  ASSERT (testS.fpa[0] (5) == 7);
  ASSERT (testS.fpa[1] (9) == 3);
  ASSERT (testS.fpb (5) == 10);

  testS.fpa[0] = foo1;
  testS.fpa[1] = foo2;
  testS.fpb = foo0;

  ASSERT (testS.fpa[0] (5) == -1);
  ASSERT (testS.fpa[1] (9) == 18);
  ASSERT (testS.fpb (5) == 7);

  testS.fpb = testS.fpa[0];
  testS.fpa[0] = testS.fpa[1];
  testS.fpa[1] = foo0;

  ASSERT (testS.fpa[0] (5) == 10);
  ASSERT (testS.fpa[1] (9) == 11);
  ASSERT (testS.fpb (5) == -1);
}
