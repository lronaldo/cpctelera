/*
   bug-1898.c 
 */

#include <testfwk.h>

struct tb {
  char c;
  short s;
  int i;
} arr[] = {
  {'a', 1200, 32000},
  {'b', 1201, 32010},
  {'c', 1202, 32020},
};

int *g0 = &(arr[1].i);
int *g1 = &(arr->i);
int *g2 = &((arr + 2)->i);

void testBug(void)
{
  int *l0 = &(arr[1].i);
  int *l1 = &(arr->i);
  int *l2 = &((arr + 2)->i);

  static int *s0 = &(arr[1].i);
  static int *s1 = &(arr->i);
  static int *s2 = &((arr + 2)->i);

  ASSERT (*g0 == 32010);
  ASSERT (*g1 == 32000);
  ASSERT (*g2 == 32020);

  ASSERT (*l0 == 32010);
  ASSERT (*l1 == 32000);
  ASSERT (*l2 == 32020);

  ASSERT (*s0 == 32010);
  ASSERT (*s1 == 32000);
  ASSERT (*s2 == 32020);
}
