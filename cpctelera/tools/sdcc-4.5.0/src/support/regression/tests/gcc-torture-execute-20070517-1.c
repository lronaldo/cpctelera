/*
   20070517-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR rtl-optimization/31691 */
/* Origin: Chi-Hua Chen <stephaniechc-gccbug@yahoo.com> */

static int get_kind(int);

static int get_kind(int v)
{
  volatile int k = v;
  return k;
}

static int some_call(void);

static int some_call(void)
{
  return 0;
}

static void example (int arg)
{
  int tmp, kind = get_kind (arg);

  if (kind == 9 || kind == 10 || kind == 5)
    {
      if (some_call() == 0)
        {
          if (kind == 9 || kind == 10)
            tmp = arg;
          else
            ASSERT (0);
        }
    }
} 

void testTortureExecute(void)
{
  example(10);
  return;
}
