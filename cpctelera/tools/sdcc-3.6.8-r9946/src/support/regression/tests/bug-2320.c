/*
   bug-2320.c z80 codegen / regalloc interaction issue overwriting register h
 */

#include <testfwk.h>

#pragma disable_warning 85

void as(char *c1, const char *c2, unsigned int i)
{
	ASSERT(0);
}

extern void p(char *c,...)
{
	ASSERT(0);
}

#define a(x) ((x) == 0 ? as(#x, __FILE__, __LINE__):(void)0)

#define BACKING_STORAGE_SIZE 8192
char *buffer;
#define buffer_size (BACKING_STORAGE_SIZE-2)
unsigned buffer_p, buffer_len;
char *buffer_gap;

void buffer_invariants()
{
	a(buffer_len <= buffer_size);
	a(buffer_p <= buffer_len);

	if (buffer_gap != buffer + buffer_size - buffer_len + buffer_p) {
		p("%04x + %04x - %04x + %04x = %04x, not %04x\n",
		  buffer, buffer_size, buffer_len, buffer_p,
		  buffer + buffer_size - buffer_len + buffer_p,
		  buffer_gap);
		a(buffer_gap == buffer + buffer_size - buffer_len + buffer_p);
	}
}

void testBug(void)
{
	buffer = (unsigned char __xdata *)(0xa5a5);
	buffer_len = 1;
	buffer_p = 0;
	buffer_gap = buffer + buffer_size - buffer_len + buffer_p;
	buffer_invariants();
}
