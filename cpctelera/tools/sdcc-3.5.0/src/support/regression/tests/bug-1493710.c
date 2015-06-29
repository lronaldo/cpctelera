/*
   bug-1493710.c

   cse
   findPointerSet with signed/unsigned operands
*/

#include <testfwk.h>

struct
  {
    unsigned char a;
  } st = { 0xff };

signed char c = -1;

unsigned char f (void)
{
  st.a += c;
  return st.a > 8;
}

void
testFindPointerSet(void)
{
  ASSERT(f() == 1);
}
