/* Tests a commit problem.
 */
#include <testfwk.h>



int foo = 16;

void f( int x )
{
  UNUSED(x);
}

void g(int bar)
{
  int a = 0;
  int b = 0;

  while(1) {
    switch(bar) {
    case 0:
      --foo;
      f(foo);
      break;
    case 1:
      ++foo;
      f(foo);
      break;
    case 2:
      ++a;
      f(a);
      break;
    case 3:
      ++b;
      f(b);
      break;
    }
  }
}

void testBug(void)
{
}
