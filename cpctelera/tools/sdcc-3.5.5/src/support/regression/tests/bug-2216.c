/*      bug-2216.c
*/

#include <testfwk.h>

#define BIG_ARRAY ((__xdata volatile unsigned char *)0x8000) // XDATA Memory

typedef __data union
{
        struct
        {
                unsigned char bit0 : 1;
                unsigned char bit1 : 1;
                unsigned char bit2 : 1;
                unsigned char bit3 : 1;
                unsigned char bit4 : 1;
                unsigned char bit5 : 1;
                unsigned char bit6 : 1;
                unsigned char bit7 : 1;
        } bits;
        unsigned char byte;
} byte_in_bit_memory;

byte_in_bit_memory __at (0x20) first_byte;

__data unsigned char __at (0x21) dummy_byte;

/* This failed with: Internal error: validateOpType failed in
   OP_SYMBOL(IC_RESULT (ic)) @ SDCCptropt.c:335: expected symbol, got value */
void foo(void)
{
        BIG_ARRAY[0] = first_byte.byte;
}

void bar(void)
{
        BIG_ARRAY[0] = dummy_byte;
}

void testBug(void)
{
        ASSERT(1);
}
