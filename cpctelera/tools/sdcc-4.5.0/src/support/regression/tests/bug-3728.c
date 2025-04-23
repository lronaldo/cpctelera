/** tests for string comparison functions, in particular for the handling of non-printable characters.
    In bug #3728, the sign waas wrong on the strcmp result for some string with characters outside the ASCII range.
*/

#include <testfwk.h>
#include <string.h>

const unsigned char s0[] = {0, 0};
const unsigned char s1[] = {1, 0};
const unsigned char s100[] = {100, 0};
const unsigned char s200[] = {200, 0};

static void
teststrcmp (void)
{
  ASSERT (strcmp(s0, s1) < 0);
  ASSERT (strncmp(s0, s1, 2) < 0);
  ASSERT (memcmp(s0, s1, 2) < 0);
  ASSERT (strcmp(s0, s100) < 0);
  ASSERT (strncmp(s0, s100, 2) < 0);
  ASSERT (memcmp(s0, s100, 2) < 0);
  ASSERT (strcmp(s0, s200) < 0);
  ASSERT (strncmp(s0, s200, 2) < 0);
  ASSERT (memcmp(s0, s200, 2) < 0);
  ASSERT (strcmp(s100, s200) < 0);
  ASSERT (strncmp(s100, s200, 2) < 0);
  ASSERT (memcmp(s100, s200, 2) < 0);
  ASSERT (strcmp(s1, s0) > 0);
  ASSERT (strncmp(s1, s0, 2) > 0);
  ASSERT (memcmp(s1, s0, 2) > 0);
  ASSERT (strcmp(s100, s0) > 0);
  ASSERT (strncmp(s100, s0, 2) > 0);
  ASSERT (memcmp(s100, s0, 2) > 0);
#if !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
  ASSERT (strcmp(s200, s1) > 0);
  ASSERT (strncmp(s200, s1, 2) > 0);
  ASSERT (memcmp(s200, s1, 2) > 0);
  ASSERT (strcmp(s200, s100) > 0);
  ASSERT (strncmp(s200, s100, 2) > 0);
  ASSERT (memcmp(s200, s100, 2) > 0);
#endif
}

