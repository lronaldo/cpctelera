/* { dg-options "-fwrapv" } */

#include <limits.h>
extern void abort ();
extern void exit (int);
void f(int i)
{
  if (i>0)
    abort();
  i = -i;
  if (i<0)
    return;
  abort ();
}

int main(void)
{
  f(INT_MIN);
  exit (0);
}
