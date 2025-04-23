/** bug-3814.c: A z80 codegen error that resulted in off-by one bugs in some unsigned char comparisons on big-endian hosts.
    isxdigit from the standard library was affected.
 */

#include <testfwk.h>

#include <ctype.h>

void testisxdigit(void)
{
	ASSERT(isxdigit('0'));
	ASSERT(isxdigit('9'));
	ASSERT(!isxdigit(':'));
	ASSERT(isxdigit('a'));
	ASSERT(isxdigit('f'));
	ASSERT(!isxdigit('g'));
	ASSERT(isxdigit('A'));
	ASSERT(isxdigit('F'));
	ASSERT(!isxdigit('G'));
}

_Bool compare(unsigned char c);

volatile char g;

void testBug(void)
{
	g = 39;
	ASSERT(compare(g));
	g = 40;
	ASSERT(!compare(g));
}

_Bool compare(unsigned char c)
{
	return(c <= 39); // less-or-equal comparison with literal <= 255 onright side, and unsigned char on the left side was affected by an off-by-onebug on big-endian hosts.
}

