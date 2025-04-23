#include "gpsim_assert.h"

unsigned char failures=0;

#if SUPPORT_BIT_TYPES
# define bit bit
#else
# define bit unsigned char
#endif

bit bit0 = 0;
bit bit1 = 0;
unsigned int aint0 = 0;
unsigned int aint1 = 0;
unsigned char achar0 = 0;
unsigned char achar1 = 0;

void
done()
{
  ASSERT(MANGLE(failures) == 0);
  PASSED();
}

void bit_invert(void)
{

  bit0 = !bit0;
  bit1 = !bit1;

  if((bit0 != bit1) || (bit0 == 0))
    failures++;
    
}

void bit_copy(void)
{

  bit0 = !bit0;
  bit1 = bit0;
}

void main(void)
{

  bit_invert();
  bit_copy();

  done();
}
