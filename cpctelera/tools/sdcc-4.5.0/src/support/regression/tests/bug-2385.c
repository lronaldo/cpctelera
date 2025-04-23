/*
   bug-2385.c
*/

#include <testfwk.h>

typedef struct fileblk *FILE;
FILE standin;

struct fileblk {
  int a;
};

int silly(void)
{
  FILE f;
  return (f = standin)->a;
}

void testBug(void)
{
  struct fileblk f;
  f.a = 42;
  standin = &f;
  ASSERT(silly() == 42);
}
