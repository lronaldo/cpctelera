/*
   bug-1654060.c

   typedef within function causes syntax error
*/

#include <testfwk.h>
typedef char mytype1;
typedef int mytype2;

mytype1 c1 = 'A';
mytype2 i1 = 12345;

void testTypedef(void)
{
  typedef int mytype1;
  typedef char mytype2;

  mytype1 i2 = 21435;
  mytype2 c2 = 'B';

  ASSERT(c1 == 'A');
  ASSERT(i1 == 12345);
  ASSERT(c2 == 'B');
  ASSERT(i2 == 21435);
}
