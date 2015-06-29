/** Test register bank allocation for the "using" keyword.
  using: no_proto, impl, both
*/
#include <testfwk.h>

#define _{using}

#if defined (__SDCC_mcs51) && defined (__SDCC_MODEL_SMALL)

  #ifndef _no_proto
    #if defined (_both)
      void my_isr (void) __interrupt (1) __using (1);
    #else
      void my_isr (void) __interrupt (1);
    #endif
  #endif

  __data char array[8];

  void
  my_isr (void) __interrupt (1) __using (1)
  {
    array[array[0]] = 1; //generate some register pressure
  }

#endif

void
testUsing (void)
{
#if defined (__SDCC_mcs51) && defined (__SDCC_MODEL_SMALL)
  ASSERT ((unsigned char)(&array[0]) >= 0x10);
  ASSERT ((unsigned char)&__numTests >= 0x10);
#endif
}
