/*
  bug2740884.c
 */

/*
 * [ 2740884 ] Incorrect assignment in array of structs
 * https://sourceforge.net/tracker/?func=detail&group_id=599&atid=100599&aid=2740884
 *
 * a bug in SDCCast.c:1883 isConformingBody()
 *  case... PTR_OP
 *  causing loopreversal to be erronously applied here
 *
 *  case... INC_OP and DEC_OP
 *    suffer the same way
 *
 * (compile "make -C .. ALL_TESTS=./tests/bug2740884.c")
 */

#include <testfwk.h>


struct {
  char Route;
} doors[4];

void foo(void) {
  char i;
  for (i = 1; i <= 2; i++)
    doors[i].Route = 0x24;
}

void bar(void) {
  char i;
  for (i = 1; i <= 2; i++)
    doors[i].Route ++;
}

void baz(void) {
  char i;
  for (i = 1; i <= 2; i++)
    doors[i].Route --;
}


void
testBug(void)
{
  doors[0].Route = 0;
  doors[1].Route = 0;
  doors[2].Route = 0;
  doors[3].Route = 0;

  foo();

  ASSERT(doors[0].Route ==    0);
  ASSERT(doors[1].Route == 0x24);
  ASSERT(doors[2].Route == 0x24);
  ASSERT(doors[3].Route ==    0);

  bar();

  ASSERT(doors[0].Route ==    0);
  ASSERT(doors[1].Route == 0x25);
  ASSERT(doors[2].Route == 0x25);
  ASSERT(doors[3].Route ==    0);

  baz();

  ASSERT(doors[0].Route ==    0);
  ASSERT(doors[1].Route == 0x24);
  ASSERT(doors[2].Route == 0x24);
  ASSERT(doors[3].Route ==    0);
}
