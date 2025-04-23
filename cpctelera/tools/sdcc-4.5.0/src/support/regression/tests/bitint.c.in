/** bit-precise integers

    width: 2, 4, 6, 7, 8, 9, 15, 16, 17, 24, 32, 33, 40, 48, 63, 64, 65
    sign: unsigned, signed
*/

#include <testfwk.h>

#include <limits.h>

// clang 11 supports bit-precise types, but deviates a bit from C23.
#if __clang_major__ == 11
#define __SDCC_BITINT_MAXWIDTH 128
#define _BitInt _ExtInt
#endif

#if __SDCC_BITINT_MAXWIDTH >= {width} // TODO: When we can regression-test in --std-c23 mode, use the standard macro from limits.h instead!

typedef {sign} _BitInt({width}) bitinttype;

bitinttype f(void) // The backend needs to support returning types of arbitrary width, not just powers of two.
{
	return 42;
}

bitinttype g(bitinttype a) // The backend needs to support parameters of arbitrary width, not just powers of two.
{
	return ++a;
}

#endif

volatile int i = 42;
volatile long long ll = 42;

void testBitInt(void)
{
#if __SDCC_BITINT_MAXWIDTH >= {width} // TODO: When we can regression-test in --std-c23 mode, use the standard macro from limits.h instead!
	ASSERT(f() == (bitinttype)42);
	ASSERT(g(0) == (bitinttype)1);

#ifndef __clang_major__ // clang 11 does not yet support addition of _BitInt of different width
#if __SDCC_BITINT_MAXWIDTH >= 4 // TODO: When we can regression-test in --std-c23 mode, use the standard macro from limits.h instead!
	ASSERT(_Generic((_BitInt(4))(4) + (_BitInt(6))(6), default: 1, _BitInt(6): 0) == 0); // _BitInt does not promote to int, even when narrower than int.
	//BUG! SDCC makes 6 char instead of int! ASSERT(_Generic((_BitInt(4))(4) + 6, default: 1, int: 0) == 0); // But it does promote to int when the other operand is int.
	//BUG! SDCC makes 6 char instead of int! ASSERT(_Generic((_BitInt(CHAR_BIT * sizeof(int)))(4) + 6, default: 1, int: 0) == 0); // Even when both are the same size.

	ASSERT(_Generic((_BitInt(4))(4) + (_BitInt(6))(300), default: 1, _BitInt(6): 0) == 0); // _BitInt does not promote to int, even when narrower than int.
	ASSERT(_Generic((_BitInt(4))(4) + 300, default: 1, int: 0) == 0); // But it does promote to int when the other operand is int.
#if __SDCC_BITINT_MAXWIDTH >= 32 // 32 should work on most platforms, including hosts with 32-bit int // TODO: When we can regression-test in --std-c23 mode, use the standard macro from limits.h instead!
	ASSERT(_Generic((_BitInt(CHAR_BIT * sizeof(int)))(4) + 300, default: 1, int: 0) == 0); // Even when both are the same size.
#endif
#endif
#endif

	bitinttype b = 1;
	ASSERT(_Generic(++b, default: 1, bitinttype: 0) == 0); // ++a is not the same a += 1, but a += (bitinttype)1.

	// Cast from int
	ASSERT((bitinttype)i == (bitinttype)42); // Explicit cast
	b = i; // Implicit cast
	ASSERT(b == (bitinttype)42);
	i = -23;
	b = i; // Implicit cast
	ASSERT(b == (bitinttype)-23);

	// Cast from long long
	ASSERT((bitinttype)ll == (bitinttype)42); // Explicit cast
	b = ll; // Implicit cast
	ASSERT(b == (bitinttype)42);

	// Casts between bit-precise types
#if {width} >= 6
	b = 0x5aff;
	unsigned _BitInt(6) b6 = b;
	ASSERT((unsigned _BitInt(6))((bitinttype)0x5aff) == b6); // Bug #3371 - both casts are done at compile time, results were different - left cast done on ast (ok), right on iCode (wrong on FreeBSD13 on aarch64).
	b = b6;
	ASSERT((bitinttype)b6 == (bitinttype)(unsigned _BitInt(6))0x5aff);
#endif
#endif
}

