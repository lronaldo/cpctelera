/*
   bug663539.c
*/

#include <testfwk.h>

#if defined (__SDCC_ds390) || defined (__SDCC_mcs51)
  volatile __xdata char __at 0x7654 x;
#endif

void
test_volatile (void)
{
#if defined (__SDCC_ds390) || defined (__SDCC_mcs51)
  //fool the compact-results.py python script
  __prints("--- Summary: 0/1/1: 0 failed of 1 tests in 1 cases.");

  x;         //this should end the simulation
  x= 's';    // emulation mode: stop be simif

  while (1); //let the "watchdog" bite
#endif
}

