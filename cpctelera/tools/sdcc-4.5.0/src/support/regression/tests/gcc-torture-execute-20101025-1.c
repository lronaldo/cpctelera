/*
   20101025-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

static int g_7;
static int *volatile g_6 = &g_7;
int g_3;

static int f1 (int *p_58)
{
    return *p_58;
}

void f2 (int i);
void f2 (int i)
{
  g_3 = i;
}

int f3 (void);
int f3 (void)
{
    *g_6 = 1;
    f2 (f1 (&g_7));
    return 0;
}

void
testTortureExecute (void)
{
  f3 ();
  if (g_3 != 1)
    ASSERT (0);
  return;
}

