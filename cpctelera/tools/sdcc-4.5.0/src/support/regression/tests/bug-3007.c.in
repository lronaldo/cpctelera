/* bug 3007
  type: char, int, long, long long
 */
#include <testfwk.h>

{type} a = 0;

void inc(void)
{
  a++;
}

void testBug(void)
{
  {type} i;
  for (i = 0; i < ({type})300; i++)
    inc();
  ASSERT (a == ({type})300);
}

{type} a1, a2;

void inc2(void)
{
  while(--a1)
    a2++;
}

void testBug2(void)
{
  a1 = 31;
  inc2();
  ASSERT(a1 == 0);
  ASSERT(a2 == 30);
}

