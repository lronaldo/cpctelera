/*
   bug-2646174.c
*/

#include <testfwk.h>

#include <string.h>

struct foo1
{
  int x;
  int y;
};

struct foo2
{
  struct foo1 base_position;
  struct foo1 direction;
};

void
f(struct foo1 *a, const struct foo2 *d)
{
  memcpy(a, &(d->direction), sizeof(struct foo1));
}

void
test_2646174(void)
{
  struct foo2 x;
  struct foo1 y;
  y.x = 0;
  x.direction.x = 1;
  f(&y, &x);
  ASSERT( y.x == 1 );
}

