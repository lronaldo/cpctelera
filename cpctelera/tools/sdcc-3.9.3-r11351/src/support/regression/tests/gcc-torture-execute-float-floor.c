/*
float-floor.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#ifdef __SDCC
#pragma disable_warning 93
#endif

#if(__SIZEOF_DOUBLE__==8)
double d = 1024.0 - 1.0 / 32768.0;
#else
double d = 1024.0 - 1.0 / 16384.0;
#endif

extern double floor(double);
extern float floorf(float);

void
testTortureExecute (void)
{
#if 0 // Enable when floor and floorf are supported in SDCC
    double df = floor(d);
    float f1 = (float)floor(d);

    if ((int)df != 1023 || (int)f1 != 1023)
      ASSERT (0);

    return;
#endif
}
