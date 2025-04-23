/** bug-3788.c: The value of a write to a bit-field in a union member was used as read value for another union member.
*/

#include <testfwk.h>

enum g_bits_defs {
    BITS_RESERVED_0 = 0x1, BITS_RESERVED_1 = 0x2,
    BITS_RESERVED_2 = 0x4, BITS_RESERVED_3 = 0x8,
    MYBIT4 = 0x10, MYBIT5 = 0x20,
    BITS_RESERVED_6 = 0x40, BITS_RESERVED_7 = 0x80 };

union {
    struct {
        unsigned char BITS_RESERVED_0 : 1;
        unsigned char BITS_RESERVED_1 : 1;
        unsigned char BITS_RESERVED_2 : 1;
        unsigned char BITS_RESERVED_3 : 1;
        unsigned char MYBIT4 : 1;
        unsigned char MYBIT5 : 1;
        unsigned char BITS_RESERVED_6 : 1;
        unsigned char BITS_RESERVED_7 : 1;
    } a;
    unsigned char is;
} g_bits;

void print_special( unsigned char c );

void send_loop_try( unsigned char c )
{
    (g_bits).a.MYBIT5 = 1;;
    if ( ((MYBIT4) & (g_bits).is) ) // The read of (g_bits).is was incorrectly optimized into a constant 1.
        print_special( c );
}

unsigned char d = 0x5a;

void testBug( void )
{
    (g_bits).a.MYBIT4 = 1;;
    send_loop_try( 0xa5 );
// The order of allocation of bit-fields within a unit
// (high-order to low-order or low-order to high-order) is
// implementation-defined. This test assumes the SDCC order,
// It fails e.g. for powerpc64-linux-gnu.
#ifdef __SDCC
    ASSERT (d == 0xa5);
#endif
}

void print_special( unsigned char c )
{
    d = c;
}

