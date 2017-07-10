#include "gpsim_assert.h"

unsigned char failures = 0;

void foo(void) {
}

void bar(int arg0) {
    (void)arg0;
}

void
done()
{
  ASSERT(MANGLE(failures) == 0);
  PASSED();
}

void main(void) {
    done();
}

