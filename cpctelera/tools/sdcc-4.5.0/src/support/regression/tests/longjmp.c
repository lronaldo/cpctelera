/** setjmp/longjmp tests (focusing on getting the return value right).
*/
#include <testfwk.h>

#if !defined(__SDCC_pic14) // Unimplemented setjmp
#include <setjmp.h>

jmp_buf buf;

void g(int v)
{
	longjmp(buf, v);
}

void testJmp(void)
{
	int r;

	r = setjmp(buf);
	if(!r)
		g(0);
	ASSERT(r == 1); // When called with an argument of 0, longjmp() makes setjmp() return 1 instead.

	r = setjmp(buf);
	if(!r)
		g(1);
	ASSERT(r == 1);
	r = setjmp(buf);
	if(!r)
		g(42);
	ASSERT(r == 42); // There used to be a bug affecting the Rabbit ports where return values other than 0/1 were wrong.

	r = setjmp(buf);
	if(!r)
		g(0x5a5);
	ASSERT(r == 0x5a5); // Ensure that we get the upper byte correct, too.
}

#endif

