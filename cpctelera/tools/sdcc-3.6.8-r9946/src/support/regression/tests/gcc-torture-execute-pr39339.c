/*
   pr39339.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <string.h>

struct C
{
  unsigned int c;
  struct D
  {
    unsigned int columns : 4;
    unsigned int fore : 12;
    unsigned int back : 6;
    unsigned int fragment : 1;
    unsigned int standout : 1;
    unsigned int underline : 1;
    unsigned int strikethrough : 1;
    unsigned int reverse : 1;
    unsigned int blink : 1;
    unsigned int half : 1;
    unsigned int bold : 1;
    unsigned int invisible : 1;
    unsigned int pad : 1;
  } attr;
};

struct A
{
  struct C *data;
  unsigned int len;
};

struct B
{
  struct A *cells;
  unsigned char soft_wrapped : 1;
};

struct E
{
  long row, col;
  struct C defaults;
};

void foo (struct E *screen, unsigned int c, int columns, struct B *row)
{
  struct D attr;
  long col;
  int i;
  col = screen->col;
  /*attr = screen->defaults.attr; struct assignment not yet supported by sdcc */
  memcpy (&attr, &(screen->defaults.attr), sizeof (struct D));
  attr.columns = columns;
  row->cells->data[col].c = c;
  /*row->cells->data[col].attr = attr; struct assignment not yet supported by sdcc */
  memcpy (&(row->cells->data[col].attr), &attr, sizeof (struct D));
  col++;
  attr.fragment = 1;
  for (i = 1; i < columns; i++)
    {
      row->cells->data[col].c = c;
      /*row->cells->data[col].attr = attr; struct assignment not yet supported by sdcc */
      memcpy (&(row->cells->data[col].attr), &attr, sizeof (struct D));
      col++;
    }
}

void
testTortureExecute (void)
{
  struct E e = {.row = 5,.col = 0,.defaults =
      {6, {-1, -1, -1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0}} };
  struct C c[4];
  struct A a = { c, 4 };
  struct B b = { &a, 1 };
  struct D d;
  memset (&c, 0, sizeof c);
  foo (&e, 65, 2, &b);
  /*d = e.defaults.attr; struct assignment not yet supported by sdcc */
  memcpy (&d, &(e.defaults.attr), sizeof (struct D));
  d.columns = 2;
  if (memcmp (&d, &c[0].attr, sizeof d))
    ASSERT (0);
  d.fragment = 1;
  if (memcmp (&d, &c[1].attr, sizeof d))
    ASSERT (0);
  return;
}


