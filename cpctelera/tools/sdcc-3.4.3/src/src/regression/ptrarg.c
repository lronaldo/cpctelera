#include "gpsim_assert.h"

// Pointer to argumet tests
#define VALUE 0x1234

unsigned char failures = 0;

void
done()
{
  ASSERT(MANGLE(failures) == 0);
  PASSED();
}

void
f2(int *p1)
{
  int t = *p1;

  if (t != VALUE)
    ++failures;

  if (*p1 != VALUE)
    ++failures;
}

void
f1(int p1)
{
  f2(&p1);
}

void
main (void)
{
  f1 (VALUE);

  done ();
}
