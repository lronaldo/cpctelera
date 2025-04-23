/*
   bug-3504.c. An expression with an optimized-out cast was handled as if the optimized-out cast had never been there.
 */

#include <testfwk.h>

unsigned int total = 0;
unsigned char prev = 255;

void m1() {
  unsigned char now = 13;
  total += (unsigned char)(now - prev); /* Code behaved as if the cast in this line was not there. */
}

void m2() {
  unsigned char now = 13;
  total += (unsigned char)(now + prev); /* Code behaved as if the cast in this line was not there. */
}

void
testBug (void)
{
	m1();
	ASSERT (total == 14);
	m2();
	ASSERT (total == 14 + 12);
}

