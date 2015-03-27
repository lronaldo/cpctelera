/*
   bug-2222.c - gave a SIGSEGV in z80 port, triggered an assertion in stm8 port.
 */

#include <testfwk.h>

typedef unsigned short u16;

static u16 vn[20];

#ifndef __SDCC_pic16
void crash1()
{
  int m = 4, n = 4;
  unsigned short *u = 0, *v = 0;
  long s = 0, i = 0;

  for (i = n - 1; i > 0; i--)
    vn[i] = (v[i] << s) | (v[i-1] >> 16-s);
  vn[0] = v[0] << s;
}

void crash2()
{
  int m = 4, n = 4;
  unsigned short *u = 0, *v = 0;
  long s = 0, i = 0;

  for (i = n - 1; i > 0; i--)
    vn[i] = (v[i] >> s) | (v[i-1] << 16-s);
  vn[0] = v[0] >> s;
}
#endif

void testBug(void)
{
}
