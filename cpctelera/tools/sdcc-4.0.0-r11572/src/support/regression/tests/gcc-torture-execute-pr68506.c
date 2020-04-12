/*
   pr68506.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* { dg-options "-fno-builtin-abort" } */

int a, b, m, n, o, p, s, u, i;
char c, q, y;
short d;
unsigned char e;
static int f, h;
static short g, r, v;
unsigned t;

int
fn1 (int p1)
{
  return a ? p1 : p1 + a;
}

unsigned char
fn2 (unsigned char p1, int p2)
{
  return p2 >= 2 ? p1 : p1 >> p2;
}

#ifndef __SDCC_pdk14 // Lack of memory
static short
fn3 ()
{
  int w, x = 0;
  for (; p < 31; p++)
    {
      s = fn1 (c | ((1 && c) == c));
      t = fn2 (s, x);
      c = (unsigned) c > -(unsigned) ((o = (m = d = t) == p) <= 4UL) && n;
      v = -c;
      y = 1;
      for (; y; y++)
	e = v == 1;
      d = 0;
      for (; h != 2;)
	{
	  for (;;)
	    {
	      if (!m)
		ASSERT (0);
	      r = 7 - f;
	      x = e = i | r;
	      q = u * g;
	      w = b == q;
	      if (w)
		break;
	    }
	  break;
	}
    }
  return x;
}
#endif

void
testTortureExecute (void)
{
#ifndef __SDCC_pdk14 // Lack of memory
  fn3 ();
  return;
#endif
}
