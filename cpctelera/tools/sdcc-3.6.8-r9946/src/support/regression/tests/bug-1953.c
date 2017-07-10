/*
   bug-1953.c
 */

#include <testfwk.h>
#include <string.h>

char pat[8] = {'S', 'D', 'C', 'C', 0, 1, 0, 0};
char str_p0[8] = "SDCC\x00\x01";
char str_p1[8] = {'S', 'D', 'C', 'C', 0, 1};

void testBug(void)
{
  char str_l0[8] = "SDCC\x00\x01";
  char str_l1[8] = {'S', 'D', 'C', 'C', 0, 1};
  static char str_s0[8] = "SDCC\x00\x01";
  static char str_s1[8] = {'S', 'D', 'C', 'C', 0, 1};

  ASSERT (memcmp (str_p0, pat, sizeof (pat)) == 0);
  ASSERT (memcmp (str_p1, pat, sizeof (pat)) == 0);
  ASSERT (memcmp (str_l0, pat, sizeof (pat)) == 0);
  ASSERT (memcmp (str_l1, pat, sizeof (pat)) == 0);
  ASSERT (memcmp (str_s0, pat, sizeof (pat)) == 0);
  ASSERT (memcmp (str_s1, pat, sizeof (pat)) == 0);
}
