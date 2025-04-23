/*      bug-2687.c
	the type of hexadecimal integer constants was in some cases decided by the rules for decimal ones.
*/

#include <testfwk.h>

#include <limits.h>

enum type {TYPE_OTHER, TYPE_INT, TYPE_LONG, TYPE_LONGLONG, TYPE_UINT, TYPE_ULONG, TYPE_ULONGLONG};

#define TYPEOF(x) _Generic(x, default: TYPE_OTHER, int: TYPE_INT, long: TYPE_LONG, long long: TYPE_LONGLONG, unsigned int: TYPE_UINT, unsigned long: TYPE_ULONG, unsigned long long: TYPE_ULONGLONG)

void testBug(void)
{
	ASSERT(0x7fffu == (unsigned int)(0x7fff));
	ASSERT(TYPEOF(0x7fffu) == TYPE_UINT);
	ASSERT(0xffffu == (unsigned int)(0xffff));
	ASSERT(TYPEOF(0xffffu) == TYPE_UINT);

	ASSERT(0x7fffffffu == (unsigned long)(0x7fffffff));
	ASSERT(TYPEOF(0x7fffffffu) == TYPE_ULONG || TYPEOF(0x7fffffffu) == TYPE_UINT);
	ASSERT(0xffffffffu == (unsigned long)(0xffffffff));
	ASSERT(TYPEOF(0xffffffffu) == TYPE_ULONG || TYPEOF(0xffffffffu) == TYPE_UINT);

	ASSERT(0x7fffffffffffffffu == (unsigned long long)(0x7fffffffffffffff));
	ASSERT(TYPEOF(0x7fffffffffffffffu) == TYPE_ULONGLONG || TYPEOF(0x7fffffffffffffffu) == TYPE_ULONG || TYPEOF(0x7fffffffffffffffu) == TYPE_UINT);
	ASSERT(0xffffffffffffffffu == (unsigned long long)(0xffffffffffffffff));
	ASSERT(TYPEOF(0xffffffffffffffffu) == TYPE_ULONGLONG || TYPEOF(0xffffffffffffffffu) == TYPE_ULONG || TYPEOF(0xffffffffffffffffu) == TYPE_UINT);
}

