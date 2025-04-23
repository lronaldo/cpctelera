/*
   bug-136564.c

   loop induction
*/

#include <testfwk.h>

volatile unsigned char p;
unsigned char i_result[3];

void
foo (void)
{
  unsigned char i;
  unsigned char u;
  static unsigned char c = '?';

  u = 0;
  while (1)
    {
      i = 0;
      switch (u)
        {
          case 0:
            /* this BBlock _case_0_0:
               - changes i
               - jumps to the successor "_swBrk_0" */
            p = 0;
            i = 42; /* fixed: Absent in "main.asm" */
            break;
          case 1:
           /* the while loop:
               - inducts i
               - has the exit block "_swBrk_0"
               sdcc inserts a new BBlock "loopExitLbl" before "_swBrk_0".
               "loopExitLbl" becomes the new successor for the exit blocks of the while loop.
               In "loopExitLbl" i can be restored without interference from
               "_swBrk_0". */
            while (c != 'x' && i < 9 )
              i++;
            break;
          default:
            p = 2;
            i = 24; /* fixed: Absent in "main.asm" */
        }
      p = i;
      i_result[u] = i;
      if (u >= 2)
        return;
      u++;
    }
}

int _strncmp (
  const char * first,
  const char * last,
  unsigned count
  )
{
  while (--count && *first && *first == *last) {
    first++;
    last++;
  }

  return( *first - *last );
}

void
testInducion(void)
{
  foo();
  ASSERT(i_result[0] == 42);
  ASSERT(i_result[1] ==  9);
  ASSERT(i_result[2] == 24);

  ASSERT(_strncmp ("SDCC is great", "SDCC is buggy", sizeof("SDCC is" )) == 0);
  ASSERT(_strncmp ("SDCC is great", "SDCC is buggy", sizeof("SDCC is ")) != 0);
}
