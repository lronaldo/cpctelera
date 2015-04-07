/* bug2817646.c
 * Structure order error
 */

#include <testfwk.h>
#include <stddef.h>

typedef struct dlnode_good {
  struct dlnode_good __xdata * nxt;
  struct dlnode_good __xdata * prv;
} dlnode_good;

typedef dlnode_good __xdata * dlist_good;

__xdata dlnode_good good = { &good, NULL };
__xdata dlist_good gl = &good;

dlnode_good __xdata *
f0 (const dlist_good * obj)
{
  return (*obj) ? (*obj)->nxt : 0;
}

dlnode_good __xdata *
f1 (__xdata dlist_good * obj)
{
  return (*obj) ? (*obj)->nxt : 0;
}

dlnode_good __xdata *
f2 (dlnode_good __xdata * const * obj)
{
  return (*obj) ? (*obj)->nxt : 0;
}

dlnode_good __xdata *
f3 (dlnode_good __xdata * __xdata * obj)
{
  return (*obj) ? (*obj)->nxt : 0;
}

typedef struct dlnode_bad {
  struct dlnode_bad __xdata * prv;
  struct dlnode_bad __xdata * nxt;
} dlnode_bad;

typedef dlnode_bad __xdata * dlist_bad;

__xdata dlnode_bad bad = { NULL, &bad };
__xdata dlist_bad bl = &bad;

dlnode_bad __xdata *
f4 (const dlist_bad * obj)
{
  return (*obj) ? (*obj)->nxt : 0;
}

dlnode_bad __xdata *
f5 (__xdata dlist_bad * obj)
{
  return (*obj) ? (*obj)->nxt : 0;
}

dlnode_bad __xdata *
f6 (dlnode_bad __xdata * const * obj)
{
  return (*obj) ? (*obj)->nxt : 0;
}

dlnode_bad __xdata *
f7 (dlnode_bad __xdata * __xdata * obj)
{
  return (*obj) ? (*obj)->nxt : 0;
}

void
testBug (void)
{
  ASSERT (f0(&gl) == &good);
  ASSERT (f1(&gl) == &good);
  ASSERT (f2(&gl) == &good);
  ASSERT (f3(&gl) == &good);

  ASSERT (f4(&bl) == &bad);
  ASSERT (f5(&bl) == &bad);
  ASSERT (f6(&bl) == &bad);
  ASSERT (f7(&bl) == &bad);
}
