/* Floats
 */
#include <testfwk.h>

const float a = 0.0;

#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
float f(void)
{
  return a * 5;
}
#endif

void testBug(void)
{
}
