/*
   960327-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdio.h>
g ()
{
  return '\n';
}

void f ()
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
  char s[] = "abcedfg012345";
  char *sp = s + 12;

  switch (g ())
    {
      case '\n':
        break;
    }

  while (*--sp == '0')
    ;
  sprintf (sp + 1, "X");

  if (s[12] != 'X')
    ASSERT (0);
#endif
}

void
testTortureExecute (void)
{
  f ();
  return;
}

