/* bug-3462.c
   Behaviour on enumeration constants outside the range of (unsigned) long didn't make sense.
 */

#include <testfwk.h>

#ifdef __SDCC
enum e0
{
	E0_0,
	E0_1
};

enum e1
{
	E1_0 = 0xff,
	E1_1
};

enum e2
{
	E2_0 = 0xffff,
	E2_1
};

enum e3
{
	E3_0 = 0xffffffff,
	E3_1
};
#endif

void
testBug (void)
{
// The choice of the underlying type is implementation-defined.
#ifdef __SDCC
	ASSERT (sizeof(enum e0) == 1);
	ASSERT (sizeof(enum e1) == 2);
	ASSERT (sizeof(enum e2) == 4);
	ASSERT (sizeof(enum e3) == 8);
#endif
}

