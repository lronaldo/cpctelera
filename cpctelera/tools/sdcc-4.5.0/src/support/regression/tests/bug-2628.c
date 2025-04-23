/*
  bug-2628.c, fetchLitPair() overwrites live IY register
*/

#include <testfwk.h>

struct list {
  struct list *next;
  void *p;
};

void *data;
static struct list *base;

static void
add(struct list *e)
{
  struct list *l;

  if(e->p != 0) {
    for(l = base; l != 0; l = l->next) {
      if(l == e) {
        e->p = data;
	return;
      }
    }
  }

  e->p = data;
}

void
testBug(void)
{
  static int d = 1234;
  struct list e;
  volatile void *tmp;

  data = &d;
  base = 0;

  add (&e);

  tmp = e.p;

  ASSERT(e.p == data);
}

