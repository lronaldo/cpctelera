/*
   bug-3240.c. A bug in z80 code generation on comparisons with 0x00 lower bytes.
 */
 
#include <testfwk.h>

#include <stdint.h>

#ifndef PORT_HOST
#pragma disable_warning 85
#endif

uint16_t p = 0x202;

int aprintf(const char *c, ...)
{
  ASSERT(0);
}

int bprintf(const char *c, ...)
{
  ASSERT(1);
}

void f(void)
{
  if (p < 0x0200)
    aprintf("1. First time it does not evaluate to TRUE\r\n");
  else
    bprintf("2. First time it does evaluate to FALSE\r\n");

  if (p < 0x0200)
    aprintf("3. But now it incorrectly evaluates to TRUE\r\n");
  else
    bprintf("4. It should have been FALSE\r\n");
}

void
testBug (void)
{
  f();
}

