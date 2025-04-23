/** bug-2516.c
*/
#include <testfwk.h>
#include <stdlib.h>
#include <math.h>

#pragma disable_warning 122

#if !defined( __SDCC_pdk14) && !defined( __SDCC_pdk15) // Not enough memory
float gfa = 1.0 / 0.0;
float gfb = (-1.0) / 0.0;
float gfc = 0.0 / 0.0;
float gfd = 1.0;

static float sfa = 1.0 / 0.0;
static float sfb = (-1.0) / 0.0;
static float sfc = 0.0 / 0.0;
static float sfd = 1.0;

static float divTest(float a, float b)
{
  return a / b;
}
#endif

void
testBug (void)
{ 
#if !defined( __SDCC_pdk14) && !defined( __SDCC_pdk15) // Not enough memory
  float lfa = 1.0 / 0.0;
  float lfb = (-1.0) / 0.0;
  float lfc = 0.0 / 0.0;
  float lfd = 1.0;

  ASSERT (isinf (gfa));
  ASSERT (isinf (sfa));
  ASSERT (isinf (lfa));

  ASSERT (isinf (gfb));
  ASSERT (isinf (sfb));
  ASSERT (isinf (lfb));

  ASSERT (isnan (gfc));
  ASSERT (isnan (sfc));
  ASSERT (isnan (lfc));

  ASSERT (!isnan (gfd));
  ASSERT (!isnan (sfd));
  ASSERT (!isnan (lfd));

  ASSERT (!isinf (gfd));
  ASSERT (!isinf (sfd));
  ASSERT (!isinf (lfd));

  ASSERT (isinf (divTest (1.0, 0.0)));
  ASSERT (isinf (divTest (-1.0, 0.0)));
  ASSERT (isnan (divTest (0.0, 0.0)));

  ASSERT (!isinf (divTest (1.0, 0.5)));
  ASSERT (!isnan (divTest (1.0, 0.5)));

  ASSERT (!isnan (divTest (3e38, 2e-38)));
  ASSERT (isinf (divTest (3e38, 2e-38)));
#endif
}
