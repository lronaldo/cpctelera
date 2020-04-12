/*
   920501-8.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when double is fully supported!
#if 0
#include <string.h>

#include <stdio.h>
#include <stdarg.h>

char buf[50];
int
va (int a, double b, int c, ...)
{
  va_list ap;
  int d, e, f, g, h, i, j, k, l, m, n, o, p;
  va_start (ap, c);

  d = va_arg (ap, int);
  e = va_arg (ap, int);
  f = va_arg (ap, int);
  g = va_arg (ap, int);
  h = va_arg (ap, int);
  i = va_arg (ap, int);
  j = va_arg (ap, int);
  k = va_arg (ap, int);
  l = va_arg (ap, int);
  m = va_arg (ap, int);
  n = va_arg (ap, int);
  o = va_arg (ap, int);
  p = va_arg (ap, int);

  sprintf (buf,
	   "%d,%f,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
	   a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p);
  va_end (ap);
}
#endif

void
testTortureExecute (void)
{
#if 0
  va (1, 1.0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
  if (strcmp ("1,1.000000,2,3,4,5,6,7,8,9,10,11,12,13,14,15", buf))
    ASSERT(0);
  return;
#endif
}

