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
    
    i = -33;
    ASSERT(0 > i);
    ASSERT(i < 0);
    ASSERT(0 >= i);
    ASSERT(i <= 0);

    i = 0;
    ASSERT(0 == i);
    ASSERT(0 <= i);
    ASSERT(0 >= i);
}

static void
testCompareConstants(void)
{
    {attr} {storage} signed {type} i;

    i = 12;
    ASSERT(i < 23);
    ASSERT(i > 3);
    ASSERT(i > -14);
    ASSERT(i <= 23);
    ASSERT(i >= 3);
    ASSERT(i >= -14);
    ASSERT(i <= 12);
    ASSERT(i >= 12);
    ASSERT(i == 12);

    i = -34;
    ASSERT(i > -126);
    ASSERT(i < -3);
    ASSERT(i < 47);
    ASSERT(i >= -126);
    ASSERT(i <= -3);
    ASSERT(i <= 47);
    ASSERT(i <= -34);
    ASSERT(i >= -34);
    ASSERT(i == -34);
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

    right = 0;
    ASSERT(left > right);
    ASSERT(left >= right);
    ASSERT(right < left);
    ASSERT(right <= left);

    right = left;
    ASSERT(left == right);
    ASSERT(left <= right);
    ASSERT(left >= right);
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
