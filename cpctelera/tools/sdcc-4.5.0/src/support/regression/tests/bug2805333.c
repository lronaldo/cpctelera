/*
   bug2805333.c
*/

#include <testfwk.h>

#ifdef __SDCC
#pragma std_sdcc99
#endif

#include <stdbool.h>

void g(char c)
{
  c;
}

bool b;

void test_bug(void)
{
  g(!b); /* Causes SDCC 2.9.0 to segfault.. */
}
