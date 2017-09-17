/*
   921117-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when sdcc supports struct!
#if 0
struct s {
  char text[11];
  int flag;
} cell;

int
check (struct s p)
{
  if (p.flag != 99)
    return 1;
  return strcmp (p.text, "0123456789");
}
#endif

void
testTortureExecute (void)
{
#if 0
  cell.flag = 99;
  strcpy (cell.text, "0123456789");

  if (check (cell))
    ASSERT (0);
  return;
#endif
}

