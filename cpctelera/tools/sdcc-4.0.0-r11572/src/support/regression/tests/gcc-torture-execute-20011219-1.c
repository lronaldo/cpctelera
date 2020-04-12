/*
   20011219-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#pragma disable_warning 84
#endif

/* This testcase failed on IA-32 at -O and above, because combine attached
   a REG_LABEL note to jump instruction already using JUMP_LABEL.  */

enum X { A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q };

void
bar (const char *x, int y, const char *z)
{
}

long
foo (enum X x, const void *y)
{
  long a;

  switch (x)
    {
    case K:
      a = *(long *)y;
      break;
    case L:
      a = *(long *)y;
      break;
    case M:
      a = *(long *)y;
      break;
    case N:
      a = *(long *)y;
      break;
    case O:
      a = *(long *)y;
      break;
    default:
      bar ("foo", 1, "bar");
    }
  return a;
}

void
testTortureExecute (void)
{
  long i = 24;
  if (foo (N, &i) != 24)
    ASSERT (0);
  return;
}

