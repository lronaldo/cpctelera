#include "gpsim_assert.h"

#if SUPPORT_BIT_TYPES
# define bit bit
#else
# define bit unsigned char
#endif

unsigned char success=0;
unsigned char failures=0;
unsigned char dummy=0;
unsigned bit bit1;

typedef unsigned char byte;

byte d2;

unsigned char uchar0 = 0xa5;

__data __at (0xa0) unsigned char uc_bank1_temp = 0x42;
__data __at (0xa2) unsigned int  ui_bank1_temp = 3;

void
done(void)
{
  dummy++;
  ASSERT(MANGLE(failures) == 0);
  PASSED();
}

void main(void)
{
  dummy = 0;
  ui_bank1_temp = 0;
  uc_bank1_temp = 0;

  bit1 = 0;

  uchar0 = (uchar0<<4) | (uchar0>>4);

  if(uchar0 > 7) {
    dummy = 8;
    uc_bank1_temp = failures;
  }

  if(uc_bank1_temp > 3)
    bit1 = 1;

  success = failures;
  done();
}
