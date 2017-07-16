#include "gpsim_assert.h"

unsigned char failures = 0;

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

void
while1 (void)
{
  unsigned char i = 10;

  do
    {
      achar0++;
    }
  while (--i);

  if (achar0 != 10)
    failures++;

}


void
main (void)
{
  while1 ();


  done ();
}
