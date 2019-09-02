/*
   980612-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

struct fd
{
	unsigned char a;
	unsigned char b;
} f = { 5 };

struct fd *g() { return &f; }
int h() { return -1; }

void
testTortureExecute (void)
{
	struct fd *f = g();
	f->b = h();
	if (((f->a & 0x7f) & ~0x10) <= 2)
		ASSERT (0);
	return;
}

