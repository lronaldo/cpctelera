#include "gpsim_assert.h"
#include "picregs.h"

/* configword.c - illustrates how the configuration word can
 * be assigned */

unsigned char failures=0;
unsigned char dummy;

#ifdef __pic14
typedef unsigned int word;
static word __at(0x2007) _config = _WDT_OFF & _PWRTE_ON;
#else /* !__pic14 */
static __code char __at(__CONFIG2L) _conf2l = _PUT_ON_2L;
static __code char __at(__CONFIG2H) _conf2h = _WDT_OFF_2H;
#endif /* !__pic14 */

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
