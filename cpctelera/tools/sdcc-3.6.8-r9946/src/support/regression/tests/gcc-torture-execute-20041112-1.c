/*
   20041112-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stddef.h>

/* This was failing on Alpha because the comparison (p != -1) was rewritten
   as (p+1 != 0) and p+1 isn't allowed to wrap for pointers.  */

int global;

static void *foo(int p)
{
  if (p == 0)
   {
      global++;
      return &global;
   }

  return (void __code *)(size_t)-1;
}

int bar(void)
{
  void *p;

  p = foo(global);
  if (p != (void __code *)(size_t)-1)
    return 1;

  global++;
  return 0;
}

void testTortureExecute(void)
{
  global = 1;
  ASSERT (bar () == 0);

  return;
}
