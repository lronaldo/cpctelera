/*
   bug-2089.c
 */

#include <testfwk.h>
#include <string.h>

struct t0_t {
  const char *p2;
  char v2[5];
};

struct t1_t {
  const char *p0;
  char v0[5];
  const char *p1;
  char v1[5];
  struct t0_t t;
};

struct t1_t w0 = {"11", "22", "33", "44", {"55", "66"}};
const struct t1_t w1 = {"11", "22", "33", "44", {"55", "66"}};

const char p0[] = {'1', '1', 0, 0, 0};
const char v0[] = {'2', '2', 0, 0, 0};
const char p1[] = {'3', '3', 0, 0, 0};
const char v1[] = {'4', '4', 0, 0, 0};
const char p2[] = {'5', '5', 0, 0, 0};
const char v2[] = {'6', '6', 0, 0, 0};

void testBug (void)
{
  ASSERT (strcmp (w0.p0, p0) == 0);
  ASSERT (strcmp (w0.p1, p1) == 0);
  ASSERT (strcmp (w0.t.p2, p2) == 0);

  ASSERT (memcmp (w0.v0, v0, sizeof (v0)) == 0);
  ASSERT (memcmp (w0.v1, v1, sizeof (v1)) == 0);
  ASSERT (memcmp (w0.t.v2, v2, sizeof (v2)) == 0);

  ASSERT (strcmp (w1.p0, p0) == 0);
  ASSERT (strcmp (w1.p1, p1) == 0);
  ASSERT (strcmp (w1.t.p2, p2) == 0);

  ASSERT (memcmp (w1.v0, v0, sizeof (v0)) == 0);
  ASSERT (memcmp (w1.v1, v1, sizeof (v1)) == 0);
  ASSERT (memcmp (w1.t.v2, v2, sizeof (v2)) == 0);
}
