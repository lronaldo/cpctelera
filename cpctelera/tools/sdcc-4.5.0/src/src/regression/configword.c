#include "gpsim_assert.h"
#include "picregs.h"

/* configword.c - illustrates how the configuration word can
 * be assigned */

unsigned char failures=0;
unsigned char dummy;

#if defined(__SDCC_pic16)
static __code char __at(__CONFIG2L) _conf2l = _PUT_ON_2L;
static __code char __at(__CONFIG2H) _conf2h = _WDT_OFF_2H;
#elif defined(__SDCC_pic14)
#  if defined(_CONFIG)
static __code unsigned int __at(_CONFIG) _config = _WDTE_OFF & _PWRTE_ON;
#  else
static __code unsigned int __at(_CONFIG1) _config1 = _WDTE_OFF & _PWRTE_ON;
#  endif
#else
#  error "Unknown PIC architecture (__SDCC_pic14 and __SDCC_pic16 undefined)"
#endif

/* TODO -- write a test that puts the PIC to sleep,
 * and verify that the WDT wakes it up */

void
done()
{
  ASSERT(MANGLE(failures) == 0);
  PASSED();
}

void main(void)
{
  dummy = 0;

  done();
}
