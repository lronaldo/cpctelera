/*
   bug663539.c
*/

#include <testfwk.h>

#if defined (__SDCC_ds390) || defined (__SDCC_mcs51)
  volatile __xdata __at 0x7654 char x;
#endif

void
test_volatile (void)
{
  ASSERT (1);

#if defined (__SDCC_ds390) || defined (__SDCC_mcs51)
  x;         //this should end the simulation

  while (1); //let the "watchdog" bite
#endif
}

