/** Swap test using 'addition' instead of usual 'or'
*/

#include <testfwk.h>
#ifdef __sun__
#include <inttypes.h>
#else
#include <stdint.h>
#endif

uint8_t 
swap_add_8(uint8_t value)
{
    value = (value << 4) + (value >> 4);
    return value;
}

#define SWAP_ADD_8(value) (((value << 4) & 0xF0u) | ((value >> 4) & 0x0Fu))
#define SWAP_ADD_8_EQUALS(value) (swap_add_8(value) == SWAP_ADD_8(value))
static void
testSwapAdd(void)
{
#ifndef __SDCC_pdk15 // Bug
    ASSERT(SWAP_ADD_8_EQUALS(0x01));
    ASSERT(SWAP_ADD_8_EQUALS(0x02));
    ASSERT(SWAP_ADD_8_EQUALS(0x04));
    ASSERT(SWAP_ADD_8_EQUALS(0x08));
    
    ASSERT(SWAP_ADD_8_EQUALS(0x10));
    ASSERT(SWAP_ADD_8_EQUALS(0x20));
    ASSERT(SWAP_ADD_8_EQUALS(0x40));
    ASSERT(SWAP_ADD_8_EQUALS(0x80));
    
    ASSERT(SWAP_ADD_8_EQUALS(0x0F));
    ASSERT(SWAP_ADD_8_EQUALS(0xF0));
    ASSERT(SWAP_ADD_8_EQUALS(0x3C));
    ASSERT(SWAP_ADD_8_EQUALS(0xC3));
#endif
}

