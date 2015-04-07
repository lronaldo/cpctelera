#include "gpsim_assert.h"
#include "picregs.h"

#pragma preproc_asm -

unsigned char failures=0;

unsigned char test_tris=0;

void
done()
{
  ASSERT(MANGLE(failures) == 0);
  PASSED();
}

void
delay_1ms(void)
{
  unsigned char cnt1m = 2;
  unsigned char cnt500u = 249;

  do {
    do {
      __asm
        nop
        nop
      __endasm;
    } while (--cnt500u > 0);
  } while (--cnt1m > 0);
}


void main(void)
{
  TRISA = 0x0f;

#if defined(__pic14)
  __asm
    BSF   STATUS,RP0
    MOVF  TRISA,W
    BCF   STATUS,RP0
    MOVWF _test_tris
  __endasm;
#else   // !defined(__pic14)
  __asm
    BANKSEL _TRISA
    MOVF    _TRISA,W
    BANKSEL _test_tris
    MOVWF   _test_tris
  __endasm;
#endif  // !defined(__pic14)

  if(test_tris != 0x0f)
    failures++;

  done();
}
