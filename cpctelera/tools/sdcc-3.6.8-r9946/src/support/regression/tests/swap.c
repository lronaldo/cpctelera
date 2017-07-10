/** various ways to swap nibbles/bytes/words
*/
#include <testfwk.h>

#define TEST_VECT_8 0x12
#define TEST_VECT_16 0x1234
#define TEST_VECT_32 0x12345678

#define SWAP_4(x) ((unsigned char)((x)<<4)|(unsigned char)((x)>>4))
    
#ifdef __SDCC
typedef unsigned int uint16;
typedef unsigned long uint32;
#else
/* on 32 and 64 bit machines */
typedef unsigned short uint16;
typedef unsigned int uint32;
#endif

typedef union {uint16 i; unsigned char c[2];} WORD;
typedef union {uint32 l; unsigned char c[4];} LONG;

static void testSwap_4(void)
{
    volatile unsigned char t=TEST_VECT_8;
    unsigned char tt;

    tt = t;
    tt = SWAP_4(tt);
    ASSERT( tt == SWAP_4(TEST_VECT_8));
}


#define SWAP_8(x) ((((x)<<8)|((x)>>8)) & 0xffff)

static void testSwap_8(void)
{
    volatile unsigned int t=TEST_VECT_16;
    unsigned int tt;
    WORD x;

    tt = t;
    tt = SWAP_8(tt);
    ASSERT( tt == SWAP_8(TEST_VECT_16));

    x.i = t;
    x.i = SWAP_8(x.i);
    ASSERT( x.i == SWAP_8(TEST_VECT_16));

#if defined (__SDCC_mcs51)
    /* this was filed as bug #1638622 (rejected) */
    x.i = t;
    x.i = x.c[1] + 256*x.c[0];
    ASSERT( x.i == SWAP_8(TEST_VECT_16));

    /* and with OR instead of ADD */
    x.i = t;
    x.i = x.c[1] | 256*x.c[0];
    ASSERT( x.i == SWAP_8(TEST_VECT_16));

    /* swapping union with little register pressure */
    {
        unsigned char tmp;
        x.i = t;

        tmp = x.c[0];
        x.c[0]=x.c[1];
        x.c[1]=tmp;

        ASSERT( x.i == SWAP_8(TEST_VECT_16));
    }
#endif
}


#define SWAP_16(x) ((((x)<<16) | ((x)>>16)) & 0xffffFFFF)

static void testSwap_16(void)
{
    volatile unsigned long t=TEST_VECT_32;
    unsigned long tt;
    LONG x;

    tt = t;
    tt = SWAP_16(tt);
    ASSERT( tt == SWAP_16(TEST_VECT_32));

    /* swapping union with little register pressure */
    {
        unsigned char c;
        x.l = t;

        c = x.c[0];
        x.c[0]=x.c[2];
        x.c[2]=c;
        c = x.c[1];
        x.c[1]=x.c[3];
        x.c[3]=c;

        ASSERT( x.l == SWAP_16(TEST_VECT_32));
    }
}

/* now for something ugly */
static void testSwap_16_ptr(void)
{
#if defined (__SDCC)
#include <sdcc-lib.h> /* just to get _AUTOMEM or _STATMEM */
#if defined (__SDCC_STACK_AUTO)
#define MY_STATIC static
#else
#define MY_STATIC
#endif
    MY_STATIC unsigned long _STATMEM tt=TEST_VECT_32;

    /* swapping with little register pressure */
    {
        unsigned char c;

        /* uglyness += 1 */
        c = *(0+(unsigned char _STATMEM *)&tt);
        *(0+(unsigned char _STATMEM *)&tt) = *(2+(unsigned char _STATMEM *)&tt);
        *(2+(unsigned char _STATMEM *)&tt) = c;
        c = *(1+(unsigned char _STATMEM *)&tt);
        *(1+(unsigned char _STATMEM *)&tt) = *(3+(unsigned char _STATMEM *)&tt);
        *(3+(unsigned char _STATMEM *)&tt) = c;
        /* uglyness -= 1 */
    }
    ASSERT( tt == SWAP_16(TEST_VECT_32));
#endif
}


static void
testSwap(void)
{
   testSwap_4();
   testSwap_8();
   testSwap_16();
   testSwap_16_ptr();
}

