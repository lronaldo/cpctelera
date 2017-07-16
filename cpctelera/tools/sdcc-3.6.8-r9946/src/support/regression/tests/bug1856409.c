/*
    bug 1856409
    storage: static __code,
*/

#include <stdint.h>
#include <testfwk.h>

#ifndef PORT_HOST
#pragma disable_warning 158 //no warning about overflow in constant (W_LIT_OVERFLOW)
#endif

typedef struct {
  unsigned int e:2;
  unsigned int f:3;
  unsigned int g:3;
} Ta;

void
testBug (void)
{
  {storage} Ta aa = {1, 29, 0};
  {storage} uint16_t xx = 100000;
  char t;

  t = aa.e;
  ASSERT (t == (1 & 3));
  t = aa.f;
  ASSERT (t == (29 & 7));
  t = aa.g;
  ASSERT (t ==  (0 & 7));

  ASSERT (xx == (uint16_t)(100000 & 65535));
}
