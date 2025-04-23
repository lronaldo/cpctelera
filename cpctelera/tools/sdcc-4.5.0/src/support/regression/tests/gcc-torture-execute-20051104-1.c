/*
   20051104-1.c from the execute part of the gcc torture suite.
 */

/*
   Optimization transformed 'if (cond) *ptr = 0;' into '*ptr = (cond) ? 0 : *ptr;'
   and thus wrote to literal string which gives undefined behavior
   or in the case of gcc a segmentation fault.
   The pointer being a plain char* and assigned a literal string is essential to the bug.
   Therefor warning 196 must be disabled.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 196
#endif

/* PR rtl-optimization/23567 */

struct
{
  int len;
  char *name;
} s;

void
testTortureExecute (void)
{
  s.len = 0;
  s.name = "";
  if (s.name [s.len] != 0)
    s.name [s.len] = 0;
  return;
}
