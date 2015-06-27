/** Interrupt tests.
    regbank: 0,1
 */
#include <testfwk.h>

#if defined(__SDCC_mcs51)

#include <8052.h>

volatile long lA = 12345;
volatile long lB = 67890;
volatile long lC = 0;

#endif

void
testInterrupt (void)
{
#if defined(__SDCC_mcs51)
  register long x = lC;

  //enable the interrupt and set it
  ET2 = 1;
  EA = 1;
  TF2 = 1;

  while (TF2)
    ; //wait until serviced
  ASSERT (lC + x == 74474);
#else
  ASSERT (1);
#endif
}

#if defined(__SDCC_mcs51)
// Timer2 interrupt service routine
// with register and (stack)spil usage
void
T2_isr (void) __interrupt 5 __using({regbank})
{
    long a, b, c;
    a = lA + 0xBABE;
    b = lB + 0xBEEB;
    c = a ^ b;
    lC = c;
    TF2 = 0;
}
#endif
