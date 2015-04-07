/*
   20060910-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#pragma disable_warning 85

/* PR rtl-optimization/28636 */
/* Origin: Andreas Schwab <schwab@suse.de> */

struct input_ty
{
  unsigned char *buffer_position;
  unsigned char *buffer_end;
};

int input_getc_complicated (struct input_ty *x) { return 0; }

int check_header (struct input_ty *deeper)
{
  unsigned len;
  for (len = 0; len < 6; len++)
    if (((deeper)->buffer_position < (deeper)->buffer_end
         ? *((deeper)->buffer_position)++
         : input_getc_complicated((deeper))) < 0)
      return 0;
  return 1;
}

struct input_ty s;
unsigned char b[6];

void testTortureExecute (void)
{
  s.buffer_position = b;
  s.buffer_end = b + sizeof b;
  if (!check_header(&s))
    ASSERT (0);
  if (s.buffer_position != s.buffer_end)
    ASSERT (0);
  return;
}
