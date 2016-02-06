/*
   bug-2403.c
*/

#include <testfwk.h>

#define CONST 0x09090909UL

typedef struct
{
  unsigned char a;
  unsigned char b;
  unsigned int  c;
  unsigned long d;
  unsigned long e;
  unsigned long f;
} struct_one;

__xdata struct_one test;
unsigned long sum;

unsigned long badd (__xdata struct_one *pointer)
{
  sum = pointer->e + pointer->f;
  if (sum)
    return CONST / sum;
  else
    return 0;
}

unsigned long add (__xdata struct_one *pointer)
{
  sum = pointer->e + pointer->f;
  return CONST / sum;
}

void testBug (void)
{
  test.e = 0x01010101;
  test.f = 0x02020202;
  ASSERT (3UL == add(&test));
}
