/** Test the comparison operators.

    type: char, int, long
    storage: static, 
    attr: volatile
 */
#include <testfwk.h>

static void
testCmpAroundZero(void)
{
    {attr} {storage} signed {type} i;

    i = 5;

    ASSERT(0 < i);
    ASSERT(i > 0);
    ASSERT(0 <= i);
    ASSERT(i >= 0);
#if !defined(__SDCC_pdk14) // Lack of memory
#if !(defined (__SDCC_pdk15) && defined(__SDCC_STACK_AUTO)) // Lack of code memory
    i = -33;
    ASSERT(0 > i);
    ASSERT(i < 0);
    ASSERT(0 >= i);
    ASSERT(i <= 0);

    i = 0;
    ASSERT(0 == i);
    ASSERT(0 <= i);
    ASSERT(0 >= i);
#endif
#endif
}

static void
testCompareConstants(void)
{
    {attr} {storage} signed {type} i;

    i = 12;
    ASSERT(i < 23);
    ASSERT(i > 3);
    ASSERT(i > -14);
#if !defined(__SDCC_pdk14) // Lack of memory
#if !(defined (__SDCC_pdk15) && defined(__SDCC_STACK_AUTO)) // Lack of code memory
    ASSERT(i <= 23);
    ASSERT(i >= 3);
    ASSERT(i >= -14);
    ASSERT(i <= 12);
    ASSERT(i >= 12);
    ASSERT(i == 12);
#endif
#endif

    i = -34;
    ASSERT(i > -126);
    ASSERT(i < -3);
#if !defined(__SDCC_pdk14) // Lack of memory
#if !(defined (__SDCC_pdk15) && defined(__SDCC_STACK_AUTO)) // Lack of code memory
    ASSERT(i < 47);
    ASSERT(i >= -126);
    ASSERT(i <= -3);
    ASSERT(i <= 47);
    ASSERT(i <= -34);
    ASSERT(i >= -34);
    ASSERT(i == -34);
#endif
#endif
}

static void
testCompareVariables(void)
{
    {attr} {storage} signed {type} left, right;

    left = 12;
    right = 47;
    ASSERT(left < right);
    ASSERT(left <= right);
    ASSERT(right > left);
    ASSERT(right >= left);

    right = -8;
    ASSERT(left > right);
    ASSERT(left >= right);
    ASSERT(right < left);
    ASSERT(right <= left);
#if !defined(__SDCC_pdk14) // Lack of memory
#if !(defined (__SDCC_pdk15) && defined(__SDCC_STACK_AUTO)) // Lack of code memory
    right = 0;
    ASSERT(left > right);
    ASSERT(left >= right);
    ASSERT(right < left);
    ASSERT(right <= left);

    right = left;
    ASSERT(left == right);
    ASSERT(left <= right);
    ASSERT(left >= right);
#endif
#endif
}

static void
testUnsignedCompare(void)
{
    {attr} {storage} unsigned {type} left, right;

    left = 0;
    right = (unsigned {type})-1;

    ASSERT(left < right);
    ASSERT(left <= right);
    ASSERT(right > left);
    ASSERT(right >= left);
}

void (*fptr)(void);
int *volatile iptr;

#if !defined(__SDCC_pdk14) // Lack of memory
signed {type} s;
unsigned {type} u;

void set ({type} v)
{
	s = v;
	u = v;
}
#endif

/* Test optimization for eliminating redundant loads in range tests. s and u should not be volatile. */
void testRange(void)
{
#if !defined(__SDCC_pdk14) // Lack of memory
#if !(defined (__SDCC_pdk15) && defined(__SDCC_STACK_AUTO)) // Lack of code memory
    set (17);

    ASSERT (s >= 17 && s <= 20);
    ASSERT (u >= 17 && u <= 20);

    set (21);

    ASSERT (!(s >= 17 && s <= 20));
    ASSERT (!(u >= 17 && u <= 20));

    set (18);

    ASSERT (s > 17 && s < 20);
    ASSERT (u > 17 && u < 20);

    set (20);

    ASSERT (!(s > 17 && s < 20));
    ASSERT (!(u > 17 && u < 20));
#endif
#endif
}

void testPointerCompare(void)
{
	int i;
	fptr = &testPointerCompare;
	iptr = &i;
	ASSERT(iptr == &i);
	ASSERT(fptr == &testPointerCompare);
}



/*
                Common cases:
                        Around zero
                        Constants on either side (reversal)
                <=
                >=
                ==
                !=
                <
                >
*/
