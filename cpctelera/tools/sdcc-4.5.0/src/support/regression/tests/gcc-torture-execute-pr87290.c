/*
   pr87290.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

/* PR middle-end/87290 */

int c;

void
f0 (void)
{
  c++;
}

int
f1 (int x)
{
  return x % 16 == 13;
}

int
f2 (int x)
{
  return x % 16 == -13;
}

void
f3 (int x)
{
  if (x % 16 == 13)
    f0 ();
}

void
f4 (int x)
{
  if (x % 16 == -13)
    f0 ();
}

void
testTortureExecute (void)
{
  int i, j;
  for (i = -30; i < 30; i++)
    {
      if (f1 (13 + i * 16) != (i >= 0) || f2 (-13 + i * 16) != (i <= 0))
	ASSERT (0);
      f3 (13 + i * 16);
      if (c != (i >= 0))
	ASSERT (0);
      f4 (-13 + i * 16);
      if (c != 1 + (i == 0))
	ASSERT (0);
      for (j = 1; j < 16; j++)
	{
	  if (f1 (13 + i * 16 + j) || f2 (-13 + i * 16 + j))
	    ASSERT (0);
	  f3 (13 + i * 16 + j);
	  f4 (-13 + i * 16 + j);
	}
      if (c != 1 + (i == 0))
	ASSERT (0);
      c = 0;
    }
  return;
}
