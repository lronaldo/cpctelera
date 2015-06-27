/*
   960326-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

struct s
{
  char a;
  int b;
  short c;
  int d[3];
  long e;
};

struct s s0 = { .b = 3, .d = {2} };
struct s static s1 = { .b = 3, .d = {2} };

void
testTortureExecute (void)
{
  struct s s2 = { .b = 3, .d = {2} };
  static struct s s3 = { .b = 3, .d = {2} };

  ASSERT (s0.a == 0 && s0.b == 3 && s0.c == 0 && s0.d[0] == 2 && s0.d[1] == 0 && s0.d[2] == 0 && s0.e == 0);
  ASSERT (s1.a == 0 && s1.b == 3 && s1.c == 0 && s1.d[0] == 2 && s1.d[1] == 0 && s1.d[2] == 0 && s1.e == 0);
  ASSERT (s2.a == 0 && s2.b == 3 && s2.c == 0 && s2.d[0] == 2 && s2.d[1] == 0 && s2.d[2] == 0 && s2.e == 0);
  ASSERT (s3.a == 0 && s3.b == 3 && s3.c == 0 && s3.d[0] == 2 && s3.d[1] == 0 && s3.d[2] == 0 && s3.e == 0);
 
  return;
}

