/* OpAssign tests
 */
#include <testfwk.h>

#ifdef __SDCC
#include <sdcc-lib.h>
#else
#define _STATMEM
#endif

typedef struct
{
  char a;
  char n;
} item_type;

item_type t;


_STATMEM item_type* get_next_item(void)
{
  /* have a side effect */
  t.n++;

  /* keep things easy, not implementing a list.
     Using a true list would break things
     even more pointedly:
     a) reading beyond end of the list and
     b) intermixing list members */
  return &t;
}


void
testOpAssign(void)
{
  t.a = 0;
  t.n = 0;

  /* get_next_item() should be called only once */
  get_next_item()->a |= 42;

  ASSERT (t.a == 42);
  ASSERT (t.n == 1);
}
