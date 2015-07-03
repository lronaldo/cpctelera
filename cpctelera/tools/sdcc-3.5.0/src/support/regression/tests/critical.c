/* Keyword "critical" tests.
 */
#include <testfwk.h>

#if defined(__SDCC_mcs51)
#include <8052.h>

typedef union
{
  unsigned int a;
  struct
  {
    unsigned char b;
    unsigned char c;
  };
} big;

//must be at least 2 bytes big and volatile
volatile big global_var = { 0 };
int y;

unsigned int
get_global (void) __critical
{
  return global_var.a;
}
#endif

void
testCritical (void)
{
#if defined(__SDCC_mcs51)
  big x;
  unsigned char i;

  //enable the interrupt and set it
  ET2 = 1;
  EA = 1;
  TF2 = 1;

  __critical x.a = global_var.a;
  ASSERT (x.b == x.c);

  x.a = get_global ();
  ASSERT (x.b == x.c);

  for (i = 10; i != 0; i--)
    {
      __critical x.a = global_var.a;
      ASSERT (x.b == x.c);

      x.a = get_global();
      ASSERT (x.b == x.c);
  }
  //check the interrupt has run at all
  ASSERT (x.a != 0);

  __critical y = 0;
  //check the interrupts are still enabled
  ASSERT (EA);
#else
  ASSERT (1);
#endif
}

#if defined(__SDCC_mcs51)
void
T2_isr (void) __interrupt 5 __using 2
{
  //do not clear flag ET2 so it keeps interrupting !
  global_var.b++;
  if (global_var.b == 0)
    global_var.b++;
  global_var.c = global_var.b;
}
#endif
