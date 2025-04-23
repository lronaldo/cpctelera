/*
   bug-3289.c
   A bug in the frontend in handling enum scope
   */

#include <testfwk.h>

void testBug(void)
{
  enum { EV1 = 3, EV2 = 13, EV3 = 7 };

  volatile int test1;
  test1 = EV2;   //this works as expected
  volatile int test2;
  test2 = EV2;   //BUG: ==> test.c:8: error 20: Undefined identifier 'EV2'
}

