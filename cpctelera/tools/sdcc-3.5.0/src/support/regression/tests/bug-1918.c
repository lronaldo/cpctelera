/*
 * bug-1918.c
 */

#include <testfwk.h>

int run_func0 (int (*f) (int), int d);
int run_func1 (int (*) (int), int);

int test_func (int x)
{
  return x > 0 ? x + 1 : x - 1;
}

int run_func0 (int (*f) (int), int d)
{
  return f(d) + 10;
}

int run_func1 (int (*f) (int), int d)
{
  return f(d) - 10;
}

void testBug (void)
{
  ASSERT (run_func0 (test_func, 23) == 34);
  ASSERT (run_func1 (test_func, -23) == -34);
}
