/*
   bug-2551.c
*/

#include <testfwk.h>
#include <stdint.h>

#if !defined( __SDCC_pdk14) && !defined( __SDCC_pdk15) // Bug #2874
volatile uint8_t ff = 0xff;

static inline uint16_t and(uint16_t a, uint16_t b) {
  uint16_t r = a & b;
  return r;
}
#endif

void testBug(void) {
#if !defined( __SDCC_pdk14) && !defined( __SDCC_pdk15) // Bug #2874
  goto cond;

cond:
  if((~((and(ff, 128) != 0)&1))&1)
    goto cond;
  else
    goto end;

end:
  ;
#endif
}

