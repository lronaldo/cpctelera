/*
   bug-2271.c
*/

#include <testfwk.h>

extern int foo0 (int *[]);
extern int foo1 (int (*[])(int), int, int);

int a = 56;
int b = 33;
int *p[2] = {&a, &b};

int foo0 (int *p[])
{
  return *p[0] + *p[1];
}

int f0 (int a)
{
  return a + 1;
}

int f1 (int b)
{
  return b << 1;
}

int (*pf[2])(int) = {f0, f1};

int foo1 (int (*pf[])(int), int a, int b)
{
  return pf[0](a) + pf[1](b);
}

void testBug (void)
{
  ASSERT (foo0 (p) == 89);
  ASSERT (foo1 (pf, 4, 8) == 21);
}
