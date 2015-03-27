/*
 * some testcases to check sanity of the offsetof implementation
 *   especially the __builtin_offsetof implementation
 */

#include <testfwk.h>
#include <stddef.h>

struct tail {
  int t;
};

struct aux {
  char x;
  char fil[2];
  struct tail ta;
};

union bla {
  char a;
  long b;
  struct aux ax;
};

struct st {
  char first;
  short a[11], b;
  struct aux s;
  struct aux arr[7];
  union bla abla;
  union bla abla_arr[2];
};

typedef struct st st_t;
typedef union bla bla_t;

#define check(type, element) \
  { int x, y; \
    union { long bits; type *ptr; } nul; nul.bits = 0;      \
    x = (int) &(nul.ptr->element); \
    y = offsetof (type, element); \
    ASSERT (x == y); \
  }

void
testOffsetOf(void)
{
  int z;

  check (struct st, first);
  check (struct st, b);
  check (struct st, a);
  check (struct st, a[9]);
  check (struct st, s);
  check (struct st, s.x);
  check (struct st, s.ta.t);
  check (struct st, s.fil);
  check (struct st, s.fil[1]);
  check (struct st, arr);
  check (struct st, arr[1]);
  check (struct st, arr[1].x);
  check (struct st, arr[1].fil);
  check (struct st, arr[1].fil[1]);
  check (struct st, abla);
  check (struct st, abla.b);
  check (struct st, abla_arr);
  check (struct st, abla_arr[1]);
  check (struct st, abla_arr[1].b);
  check (struct st, abla_arr[1].ax);
  check (struct st, abla_arr[1].ax.fil);
  check (struct st, abla_arr[1].ax.fil[1]);

  z = 7; check (struct st, a[z*3+1]);
  z = 3; check (struct st, arr[z].x);

  check (st_t, arr[1].fil[1]);
  check (bla_t, b);

  ASSERT (0 == offsetof (union bla, b));
  ASSERT (0 == offsetof (bla_t, b));
}
