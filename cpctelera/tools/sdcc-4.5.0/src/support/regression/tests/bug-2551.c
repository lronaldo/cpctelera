/*
   bug-2551.c
*/

#include <testfwk.h>
#include <stdint.h>

volatile uint8_t ff = 0xff;

static inline uint16_t and(uint16_t a, uint16_t b) {
  uint16_t r = a & b;
  return r;
}

void testBug(void) {
  goto cond;

cond:
  if((~((and(ff, 128) != 0)&1))&1)
    goto cond;
  else
    goto end;

end:
  ;
}

