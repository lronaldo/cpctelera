/*
   bug1426356.c
*/

#include <testfwk.h>

const union pu {
  unsigned char t1;
  unsigned char t2;
} tst[2] = {{ 1 }, { 2 }};

void
test_1426356(void)
{
  ASSERT( tst[0].t1 == 1 );
  ASSERT( tst[0].t2 == 1 );
  ASSERT( tst[1].t1 == 2 );
  ASSERT( tst[1].t2 == 2 );
}
