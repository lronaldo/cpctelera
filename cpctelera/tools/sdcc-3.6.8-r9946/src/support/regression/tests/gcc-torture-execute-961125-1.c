/*
   961125-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 196
#endif

static char *
begfield (int tab, char *ptr, char *lim, int sword, int schar)
{
  if (tab)
    {
      while (ptr < lim && sword--)
        {
          while (ptr < lim && *ptr != tab)
            ++ptr;
          if (ptr < lim)
            ++ptr;
        }
    }
  else
    {
      while (1)
        ;
    }

  if (ptr + schar <= lim)
    ptr += schar;

  return ptr;
}

void
testTortureExecute (void)
{
  char *s = ":ab";
  char *lim = s + 3;
  if (begfield (':', s, lim, 1, 1) != s + 2)
    ASSERT (0);
  return;
}
