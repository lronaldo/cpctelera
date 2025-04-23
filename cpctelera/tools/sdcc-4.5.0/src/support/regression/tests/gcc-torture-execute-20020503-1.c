/*
   20020503-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if !defined(__SDCC_mcs51) && !defined(__SDCC_pdk14) && !defined(__SDCC_pic14) // Lack of memory
/* PR 6534 */
/* GCSE unified the two i<0 tests, but if-conversion to ui=abs(i) 
   insertted the code at the wrong place corrupting the i<0 test.  */

static char *
inttostr (long i, char buf[128])
{
  unsigned long ui = i;
  char *p = buf + 127;
  *p = '\0';
  if (i < 0)
    ui = -ui;
  do
    *--p = '0' + ui % 10;
  while ((ui /= 10) != 0);
  if (i < 0)
    *--p = '-';
  return p;
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_mcs51) && !defined(__SDCC_pdk14) && !defined(__SDCC_pic14) // Lack of memory
  char buf[128], *p;

  p = inttostr (-1, buf);
  if (*p != '-')
    ASSERT (0);
  return;
#endif
}
