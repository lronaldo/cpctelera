/*
   bug-2059.c
 */

#include <testfwk.h>
#include <string.h>

char pg0[] = "SDCC";
char pg1[] = {"SDCC"};

void testBug (void)
{
  static char ps0[] = "sdcc";
  static char ps1[] = {"sdcc"};
  char pa0[] = "benshi";
  char pa1[] = {"benshi"};

  ASSERT (strcmp (pg0, pg1) == 0);
  ASSERT (strcmp (ps0, ps1) == 0);
  ASSERT (strcmp (pa0, pa1) == 0);
}
