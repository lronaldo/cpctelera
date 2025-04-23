/* bug 3048
   stm8 and pdk code generator hang on shift by negative literal
   type: char, signed char, unsigned char, signed int, unsigned int, signed long, unsigned long, signed long long, unsigned long long
 */
#include <testfwk.h>

#pragma disable_warning 259

void g(void)
{
}

void fr({type} x)
{
  if (((x >> -1))) // Undefined behaviour, but we should be fine if this is never executed at runtime.
    g();
}

void fl({type} x)
{
  if (((x << -1))) // Undefined behaviour, but we should be fine if this is never executed at runtime.
    g();
}

void testBug(void)
{
  volatile _Bool b = 0;
  {type} i = 0x5a;
  if (b)
    fr(i);
  if (b)
    fl(i);
}

