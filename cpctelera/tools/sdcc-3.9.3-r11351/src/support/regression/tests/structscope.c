/* Test struct scoping rules

 */

#include <testfwk.h>

/* declare an incomplete struct type */
struct tag2;

struct tag1 {
  struct tag1 *mykind;
  unsigned char x;
  unsigned char y;
  struct tag2 *other;
} s1;

/* complete the previously incomplete type */
struct tag2 {
  unsigned char z;
  struct tag1 *other;
} s2;

void
test_global(void)
{
  s1.mykind = &s1;
  s1.other = &s2;
  s2.other = &s1;
  s1.x = 1;
  s1.y = 2;
  s2.z = 3;
  ASSERT(s2.other->x == 1);
  ASSERT(s1.other->z == 3);
  ASSERT(s1.other->other->y == 2);
  ASSERT(s1.mykind->y == 2);
}

void
test_nested(void)
{
  {
    struct tag1 {
      unsigned char a;
    } ls1;
    ls1.a=1;
  }
  {
    struct tag1 ls2; /* should bind to global level tag1 */
    ls2.x=2;
  }
  {
    struct tag1 {
      unsigned char b;
      struct tag2 *globals2; /* should bind to global level tag2 */
    } ls3;
    ls3.b = 3;
    ls3.globals2 = &s2;
  }
  {
    struct tag2; /* incomplete local tag */
    struct tag1 {
      struct tag2 *locals2; /* should bind to local level s2 tag */
    } ls4;
    struct tag2 {
      unsigned char c;
    } ls5;

    ls4.locals2 = &ls5;
    ls4.locals2->c = 9;
  }
  s1.x = 0;
} 
