/*
whatever
*/

#include <testfwk.h>

volatile char c[1];

#define INVOKE_1(X) X
#define INVOKE_2(X) INVOKE_1(X) INVOKE_1(X)
#define INVOKE_4(X) INVOKE_2(X) INVOKE_2(X)
#define INVOKE_8(X) INVOKE_4(X) INVOKE_4(X)
#define INVOKE_16(X) INVOKE_8(X) INVOKE_8(X)
#define INVOKE_32(X) INVOKE_16(X) INVOKE_16(X)
#define INVOKE_64(X) INVOKE_32(X) INVOKE_32(X)
#define INVOKE_128(X) INVOKE_64(X) INVOKE_64(X)
#define INVOKE_256(X) INVOKE_128(X) INVOKE_128(X)
#define INVOKE_512(X) INVOKE_256(X) INVOKE_256(X)
#define INVOKE_1024(X) INVOKE_512(X) INVOKE_512(X)
#define INVOKE_2048(X) INVOKE_1024(X) INVOKE_1024(X)

void
testBug (void)
{
#if 0 // Bug  no yet fixed
#if !defined(__SDCC_pdk14) // Lack of memory
	INVOKE_2048(c[0] = 0;)
#endif
#endif
}

