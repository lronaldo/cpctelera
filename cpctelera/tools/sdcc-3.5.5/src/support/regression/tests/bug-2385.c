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
#if !defined(__SDCC_mcs51) && !defined(__SDCC_z80) && !defined(__SDCC_z180) && !defined(__SDCC_r2k) && !defined(__SDCC_r3ka) && !defined(__SDCC_gbz80) && !defined(__SDCC_tlcs90)
  struct fileblk f;
  f.a = 42;
  standin = &f;
  ASSERT(silly() == 42);
#endif
}

