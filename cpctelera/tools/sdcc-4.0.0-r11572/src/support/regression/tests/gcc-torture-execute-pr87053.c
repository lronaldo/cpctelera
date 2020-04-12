/*
   pr87053.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#include <string.h>

/* PR middle-end/87053 */

#if 0
const union
{ struct {
    char x[4];
    char y[4];
  };
  struct {
    char z[8];
  };
} u = {{"1234", "567"}};
#endif

void
testTortureExecute (void)
{
#if 0 // Bug
  if (strlen (u.z) != 7)
    ASSERT (0);
#endif
}

