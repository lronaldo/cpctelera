/** Operations with 1-bit sized bitfields
    preBits: 0, 8, 16
    pattern: 0, 1
*/

#pragma disable_warning 88

#include <testfwk.h>
#ifdef __sun__
#include <inttypes.h>
#else
#include <stdint.h>
#endif

#include <stdbool.h>

#define PRE_BITS ({preBits})

typedef struct
{
    #if PRE_BITS > 0
        unsigned int preBits   : PRE_BITS;
    #endif
    unsigned int bit0       : 1;
    unsigned int bit1       : 1;
    unsigned int bit2       : 1;
    unsigned int bit3       : 1;
    unsigned int bit4       : 1;
    unsigned int bit5       : 1;
    unsigned int bit6       : 1;
    unsigned int bit7       : 1;
    
    unsigned int postBits   : 8;
}struct_8bits;


#if PATTERN == 0
    #define PRE_BIT_VALUE 0
    #define POST_BIT_VALUE 0
#else
    #define PRE_BIT_VALUE ((1UL << PRE_BITS) - 1)
    #define POST_BIT_VALUE 0xFF
#endif

bool
bits_check_value_no_cast(const struct_8bits * var, const uint8_t value)
{
    if((var->bit0 > 0) != ((value & 0x01) > 0)) return false;
    if((var->bit1 > 0) != ((value & 0x02) > 0)) return false;
    if((var->bit2 > 0) != ((value & 0x04) > 0)) return false;
    if((var->bit3 > 0) != ((value & 0x08) > 0)) return false;
        
    if((var->bit4 > 0) != ((value & 0x10) > 0)) return false;
    if((var->bit5 > 0) != ((value & 0x20) > 0)) return false;
    if((var->bit6 > 0) != ((value & 0x40) > 0)) return false;
    if((var->bit7 > 0) != ((value & 0x80) > 0)) return false;
    
    return true;
}


#define CAST_TO_UINT8(x) ((uint8_t)(x))

// Lack of memory.
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15)

bool
bits_check_value(const struct_8bits * const var, const uint8_t value)
{
    #if PRE_BITS > 0
        if(var->preBits != PRE_BIT_VALUE) return false;
    #endif

    if(var->postBits != POST_BIT_VALUE) return false;
    
    if((var->bit0 > 0) != (CAST_TO_UINT8(value & 0x01) > 0)) return false;
    if((var->bit1 > 0) != (CAST_TO_UINT8(value & 0x02) > 0)) return false;
    if((var->bit2 > 0) != (CAST_TO_UINT8(value & 0x04) > 0)) return false;
    if((var->bit3 > 0) != (CAST_TO_UINT8(value & 0x08) > 0)) return false;
        
    if((var->bit4 > 0) != (CAST_TO_UINT8(value & 0x10) > 0)) return false;
    if((var->bit5 > 0) != (CAST_TO_UINT8(value & 0x20) > 0)) return false;
    if((var->bit6 > 0) != (CAST_TO_UINT8(value & 0x40) > 0)) return false;
    if((var->bit7 > 0) != (CAST_TO_UINT8(value & 0x80) > 0)) return false;
    
    return bits_check_value_no_cast(var, value);
}

void 
bits_set_value(struct_8bits * const var, const uint8_t value)
{
    #if PRE_BITS > 0
        var->preBits = PRE_BIT_VALUE;
    #endif
    
    var->postBits = POST_BIT_VALUE;
    
    var->bit0 = CAST_TO_UINT8(value & 0x01) > 0;
    var->bit1 = CAST_TO_UINT8(value & 0x02) > 0;
    var->bit2 = CAST_TO_UINT8(value & 0x04) > 0;
    var->bit3 = CAST_TO_UINT8(value & 0x08) > 0;
        
    var->bit4 = CAST_TO_UINT8(value & 0x10) > 0;
    var->bit5 = CAST_TO_UINT8(value & 0x20) > 0;
    var->bit6 = CAST_TO_UINT8(value & 0x40) > 0;
    var->bit7 = CAST_TO_UINT8(value & 0x80) > 0;
}
#endif

#define BITS_SET_AND_CHECK(value) bits_set_value(&volatileBits, value); ASSERT(bits_check_value(&volatileBits, value))

static void
testBitfields(void)
{
// Lack of memory.
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15)
    volatile struct_8bits volatileBits;

    BITS_SET_AND_CHECK(0x00);
    BITS_SET_AND_CHECK(0xFF);
    
    BITS_SET_AND_CHECK(0x0F);
    BITS_SET_AND_CHECK(0xF0);
    
    BITS_SET_AND_CHECK(0x01);
    BITS_SET_AND_CHECK(0x02);
    BITS_SET_AND_CHECK(0x04);
    BITS_SET_AND_CHECK(0x08);
    
    BITS_SET_AND_CHECK(0x10);
    BITS_SET_AND_CHECK(0x20);
    BITS_SET_AND_CHECK(0x40);
    BITS_SET_AND_CHECK(0x80);
#endif
}

