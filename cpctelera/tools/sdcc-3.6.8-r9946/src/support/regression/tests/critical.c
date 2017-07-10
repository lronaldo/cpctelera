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

#if defined(__SDCC_mcs51) || defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_r2k) || defined(__SDCC_r3ka) || defined(__SDCC_hc08) || defined(__SDCC_s08) || defined(__SDCC_tlcs90) || defined(__SDCC_stm8)
// Check that param offsets are correctly adjusted for critial functions
long critical_function(long a, long b, long c) __critical
{
  if (a > 10L)
    return a + b + c;
  else if (b < 10L)
    return a - b - c;
  else
    return a + b - c;
}
inline long critical_function_inline(long a, long b, long c) __critical
{
  if (a > 10L)
    return a + b + c;
  else if (b < 10L)
    return a - b - c;
  else
    return a + b - c;
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

#if defined(__SDCC_mcs51) || defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_r2k) || defined(__SDCC_r3ka) || defined(__SDCC_hc08) || defined(__SDCC_s08) || defined(__SDCC_tlcs90) || defined(__SDCC_stm8)
  ASSERT(critical_function(11, 1, 1) == 13);
  ASSERT(critical_function(5, 1, 1) == 3);
  ASSERT(critical_function(5, 11, 1) == 15);
  ASSERT(critical_function_inline(11, 1, 1) == 13);
  ASSERT(critical_function_inline(5, 1, 1) == 3);
  ASSERT(critical_function_inline(5, 11, 1) == 15);
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
