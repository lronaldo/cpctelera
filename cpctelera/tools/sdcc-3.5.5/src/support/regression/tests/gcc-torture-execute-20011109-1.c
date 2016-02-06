/*
   20011109-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void fail1(void)
{
  ASSERT (0);
}
void fail2(void)
{
  ASSERT (0);
}
void fail3(void)
{
  ASSERT (0);
}
void fail4(void)
{
  ASSERT (0);
}


void foo(long x)
{
  switch (x)
    {
    case -6: 
      fail1 (); break;
    case 0: 
      fail2 (); break;
    case 1: case 2: 
      break;
    case 3: case 4: case 5: 
      fail3 ();
      break;
    default:
      fail4 ();
      break;
    }
  switch (x)
    {
      
    case -3: 
      fail1 (); break;
    case 0: case 4: 
      fail2 (); break;
    case 1: case 3: 
      break;
    case 2: case 8: 
      ASSERT (0);
      break;
    default:
      fail4 ();
      break;
    }
}

void
testTortureExecute (void)
{
  foo (1);
  return;
}

