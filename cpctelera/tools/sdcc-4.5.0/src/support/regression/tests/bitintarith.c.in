/** bit-precise integers

    width: 2, 4, 7, 8, 9, 15, 16, 17, 24, 32, 33, 40, 48, 63, 64, 65
*/

#include <testfwk.h>

// clang 11 supports bit-precise types, but deviates a bit from C23.
#if __clang_major__ == 11
#define __SDCC_BITINT_MAXWIDTH 128
#define _BitInt _ExtInt
#endif

#if __SDCC_BITINT_MAXWIDTH >= {width} // TODO: When we can regression-test in --std-c23 mode, use the standard macro from limits.h instead!
#if {width} <= 40 || !defined (__SDCC_pdk14) // Lack of memory

typedef _BitInt({width}) bitinttype;

typedef unsigned _BitInt({width}) ubitinttype;

volatile bitinttype a, b;
volatile ubitinttype ua, ub;
volatile char c = 1;

#endif
#endif

void testBitIntArith(void)
{
#ifndef __clang_major__ // clang 11 doesn't support == between _BitInt and int
#if __SDCC_BITINT_MAXWIDTH >= {width} // TODO: When we can regression-test in --std-c23 mode, use the standard macro from limits.h instead!
#if ({width} <= 40 || !defined (__SDCC_pdk14)) && ({width} <= 48 || !defined (__SDCC_pdk15)) // Lack of memory

	a = -1, b = 1;
	ASSERT(a + b == 0);
	ua = 1, ub = 1;
	ASSERT(ua + ub == 2);

	// From here on, we do the computation on the left at runtime, the one on the right at compile time, to check both.
	a = 3, ua = 3;
#if {width} >= 4
	ASSERT(a << 1 == (bitinttype)(3) << 1);
	ASSERT(ua << 1 == (ubitinttype)(3) << 1);
	ASSERT(ua << 3 == (ubitinttype)(3) << 3);
#endif
	ASSERT(a >> 1 == (bitinttype)(3) >> 1);
	ASSERT(ua >> 1 == (ubitinttype)(3) >> 1);
	ua = 0xffffffff;
	ASSERT(ua << 1 == (ubitinttype)(0xffffffff) << 1);
	ASSERT(ua >> 1 == (ubitinttype)(0xffffffff) >> 1);
	ASSERT(ua << c == (ubitinttype)(0xffffffff) << 1);
	ASSERT(ua >> c == (ubitinttype)(0xffffffff) >> 1);

	a = 23, b = -42;
	ua = 23, ub = -42;

#if {width} >= 8 // Signed overflow is undefined behaviour	
	ASSERT(a + b == (bitinttype)(23) + (bitinttype)(-42));
	ASSERT(a - b == (bitinttype)(23) - (bitinttype)(-42));
#endif

	// Wrap on unsigned overflow
	ASSERT(ua + ub == (ubitinttype)(23) + (ubitinttype)(-42));
	ASSERT(ua - ub == (ubitinttype)(23) - (ubitinttype)(-42));

#if !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory

#if {width} >= 11 // Signed overflow is undefined behaviour
	ASSERT(a * b == (bitinttype)(23) * (bitinttype)(-42));
#endif

#if {width} >= 8 // Signed overflow is undefined behaviour	
	ASSERT(a % b == (bitinttype)(23) % (bitinttype)(-42));
	ASSERT(a / b == (bitinttype)(23) / (bitinttype)(-42));
#endif

	ASSERT(ua * ub == (ubitinttype)(23) * (ubitinttype)(-42));
	ASSERT(ua % ub == (ubitinttype)(23) % (ubitinttype)(-42));
	ASSERT(ua / ub == (ubitinttype)(23) / (ubitinttype)(-42));

#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) // Bug #3355
	ASSERT((a < b) == ((bitinttype)(23) < (bitinttype)(-42)));
	ASSERT((a <= b) == ((bitinttype)(23) <= (bitinttype)(-42)));
	ASSERT((ua < ub) == ((ubitinttype)(23) < (ubitinttype)(-42)));
	ASSERT((ua <= ub) == ((ubitinttype)(23) <= (ubitinttype)(-42)));
#endif

	ASSERT((ua & ub) == ((ubitinttype)(23) & (ubitinttype)(-42)));
	ASSERT((ua | ub) == ((ubitinttype)(23) | (ubitinttype)(-42)));
	ASSERT((ua ^ ub) == ((ubitinttype)(23) ^ (ubitinttype)(-42)));

	// Test increment / decrement and it wraparound for unsigned _BitInt.
	ua = 1;
	ASSERT(--ua == 0);
	ASSERT(--ua == (ubitinttype)0xffffffffffffffffull);
	ASSERT(++ua == 0);
#endif
#endif
#endif
#endif
}

