/* saving "bits" test for mcs51/stack-auto.
 */
#include <testfwk.h>

#if defined(__SDCC_mcs51)
#include <8052.h>
#include <stdbool.h>

bool
manipulate_bits (bool x) __using 2
{
  return x;
}

bool
complement (bool x)
{
  return !x;
}
#endif

void
testSaveBits (void)
{
#if defined(__SDCC_mcs51)
  //enable the interrupt and set it
  ET2 = 1;
  EA = 1;
  TF2 = 1;

  //this will pass b0 cleared, test whether it will arrive cleared
  if (complement(false))
    {
      EA = 0;
      ASSERT (1);
    }
  else
    {
      EA = 0;
      ASSERT (0);
    }
#else
  ASSERT (1);
#endif
}

#if defined(__SDCC_mcs51)
void
T2_isr (void) __interrupt 5 __using 2
{
  //do not clear flag ET2 so it keeps interrupting !

  //this will set b0
  manipulate_bits (true);
}
#endif

