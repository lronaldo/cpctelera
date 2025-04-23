/*
   bug-3023.c. SDCC failed to compile an attempt to write to a string literal.
 */
 
#include <testfwk.h>

#pragma disable_warning 293

void f(void)
{
  *(char *)"c" = 0; // Undefined behaviour if this ever gets executed. Should still compile, though.
}

void
testBug (void)
{
}

