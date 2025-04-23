/* PR tree-optimization/53465 */

/* suppress warning W_LOCAL_NOINIT, c is not used uninitialized,
   but that is hard to spot for the compiler */
#pragma disable_warning 84

extern void abort (void);

static const int a[] = { 1, 2 };

void
foo (const int *x, int y)
{
  int i;
  int b = 0;
  int c;
  for (i = 0; i < y; i++)
    {
      int d = x[i];
      if (d == 0)
	break;
      if (b && d <= c)
	abort ();
      c = d;
      b = 1;
    }
}

int
main (void)
{
  foo (a, 2);
  return 0;
}
