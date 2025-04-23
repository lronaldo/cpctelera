/* bug-2859.c
   Cannot compare function to 0.
 */

#include <testfwk.h>

void
a(void)
{
  /* empty */
}

typedef void (*vf)(void);

void
testBug(void)
{
  /* previously broken */
  ASSERT(a != 0);
  ASSERT(a != ((void *)0));
  ASSERT(&a != 0);
  ASSERT(&a != ((void *)0));
  ASSERT(!!a);
  /* previously working */
  ASSERT((vf)a);
  ASSERT(((void *)a) != 0);
  ASSERT((_Bool)a);
}
