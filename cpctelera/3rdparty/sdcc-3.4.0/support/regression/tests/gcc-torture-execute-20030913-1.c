/*
   20030913-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Assignments via pointers pointing to global variables were being killed
   by SSA-DCE.  Test contributed by Paul Brook <paul@nowt.org>  */

int glob; 
 
void 
fn2(int ** q) 
{ 
  *q = &glob; 
} 
 
void test() 
{ 
  int *p; 
 
  fn2(&p); 
 
  *p=42; 
} 
 
void
testTortureExecute (void)
{ 
  test(); 
  if (glob != 42) ASSERT(0); 
  return; 
}

