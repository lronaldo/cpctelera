/*
   pr15262.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c11
#pragma disable_warning 85
#endif

/* PR tree-optimization/20601 */
/*extern void abort (void);
extern void exit (int);*/

struct T
{
  const char *t1;
  char t2[4096];
  char **t3;
};

#if defined (__SDCC) && !defined (__SDCC_mcs51)
int a[5];
int b;
const char **c;
int d;
const char **e;
struct T t;
char *f[16];
const char *g[] = { "a", "-u", "b", "c" };

_Noreturn void
foo (void)
{
  while (1);
}

char *
bar (char *x, unsigned int y)
{
  return 0;
}

static inline char *
baz (char *x, unsigned int y)
{
  if (sizeof (t.t2) != (unsigned int) -1 && y > sizeof (t.t2))
    foo ();
  return bar (x, y);
}

static inline int
setup1 (int x)
{
  char *p;
  int rval;

  if (!baz (t.t2, sizeof (t.t2)))
    baz (t.t2, sizeof (t.t2));

  if (x & 0x200)
    {
      char **h, **i = e;

      ++d;
      e = f;
      if (t.t1 && *t.t1)
        e[0] = t.t1;
      else
        ASSERT (0);

      for (h = e + 1; (*h = *i); ++i, ++h)
        ;
    }
  return 1;
}

static inline int
setup2 (void)
{
  int j = 1;

  e = c + 1;
  d = b - 1;
  while (d > 0 && e[0][0] == '-')
    {
      if (e[0][1] != '\0' && e[0][2] != '\0')
        ASSERT (0);

      switch (e[0][1])
        {
        case 'u':
          if (!e[1])
            ASSERT (0);

          t.t3 = &e[1];
          d--;
          e++;
          break;
        case 'P':
          j |= 0x1000;
          break;
        case '-':
          d--;
          e++;
          if (j == 1)
            j |= 0x600;
          return j;
        }
      d--;
      e++;
    }

  if (d > 0 && !(j & 1))
    ASSERT (0);

  return j;
}
#endif

void
testTortureExecute (void)
{
#if defined (__SDCC) && !defined (__SDCC_mcs51)
  int x;
  c = g;
  b = 4;
  x = setup2 ();
  t.t1 = "/bin/sh";
  setup1 (x);
  /* PRE shouldn't transform x into the constant 0x601 here, it's not legal.  */
  if ((x & 0x400) && !a[4])
    ASSERT (0);
  return;
#endif
}

