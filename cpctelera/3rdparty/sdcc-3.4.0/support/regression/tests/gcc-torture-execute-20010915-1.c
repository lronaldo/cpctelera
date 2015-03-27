/*
   20010915-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#endif

#include <string.h>

/* Bug in reorg.c, deleting the "++" in the last loop in main.
   Origin: <hp@axis.com>.  */

extern void f (void);
extern int x (int, const char **);
extern int r (const char *);
extern char *s (const char *, const char **);
extern char *m (const char *);
char *u;
char *h;
int check = 0;
int o = 0;

void
testTortureExecute (void)
{
  const char *args[] = {"a", "b", "c", "d", "e"};
  if (x (5, args) != 0 || check != 2 || o != 5)
    ASSERT (0);
  return;
}

int x (int argc, const char **argv)
{
  int opt = 0;
  char *g = 0;
  const char *p = 0;

  if (argc > o && argc > 2 && argv[o])
    {
      g = s (argv[o], &p);
      if (g)
	{
	  *g++ = '\0';
	  h = s (g, &p);
	  if (g == p)
	    h = m (g);
	}
      u = s (argv[o], &p);
      if (argv[o] == p)
	u = m (argv[o]);
    }
  else
    ASSERT (0);

  while (++o < argc)
    if (r (argv[o]) == 0)
      return 1;

  return 0;
}

char *m (const char *x) { ASSERT (0); return 0;}
char *s (const char *v, const char **pp)
{
  if (strcmp (v, "a") != 0 || check++ > 1)
    ASSERT (0);
  *pp = v+1;
  return 0;
}

int r (const char *f)
{
  static char c[2] = "b";
  static int cnt = 0;

  if (*f != *c || f[1] != c[1] || cnt > 3)
    ASSERT (0);
  c[0]++;
  cnt++;
  return 1;
}

