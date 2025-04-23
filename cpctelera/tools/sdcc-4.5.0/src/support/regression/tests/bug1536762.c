/*
   bug1536762.c
*/

#include <testfwk.h>
#include <string.h>
#include <stdint.h>

__xdata uint8_t c = 1;

struct d {
  __xdata struct d *n;
  uint8_t f;
  uint8_t s;
  __xdata uint8_t *buffer;
  uint16_t length;
};


__xdata struct d xd = {&xd, 1, 0xab, &c, 3};


struct {
  __xdata struct d *c;
  int16_t count;
  __xdata uint8_t *bptr;
} s = {&xd, -1, &c};


void
blurb (void)
{
  if (s.count < 0)
    {
      s.c->s = 0xef;
      s.count = s.c->length - 1;
      s.bptr = s.c->buffer;
    }
  *s.bptr = 0;
  s.bptr++;
  s.count--;
}


void 
testBug (void)
{
  ASSERT (xd.s == 0xab);
  ASSERT (s.c->s == 0xab);
  
  s.c->s = 0xcd;

  ASSERT (xd.s == 0xcd);
  ASSERT (s.c->s == 0xcd);

  blurb ();

  ASSERT (xd.s == 0xef);
  ASSERT (s.c->s == 0xef);
  ASSERT (c == 0);
}
