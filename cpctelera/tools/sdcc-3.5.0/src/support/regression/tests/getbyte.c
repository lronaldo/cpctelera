/* getbyte/getword
    sign: unsigned, signed
*/

#include <testfwk.h>
#include <stdint.h>

{sign} long global = ({sign} long)0x12345678;

void testGet(void)
{
    ASSERT (((global >>  0) & 0xFF) == 0x78);               // getByte 0
    ASSERT (((global >>  8) & 0xFF) == 0x56);               // getByte 1
    ASSERT (((global >> 16) & 0xFF) == 0x34);               // getByte 2
    ASSERT (((global >> 24) & 0xFF) == 0x12);               // getByte 3

    ASSERT (((global >>  0) & 0xFFFF) == 0x5678);           // getWord 0
    ASSERT (((global >>  8) & 0xFFFF) == 0x3456);           // getWord 1
    ASSERT (((global >> 16) & 0xFFFF) == 0x1234);           // getWord 2

    ASSERT ((uint8_t)(global >>  0) == (uint8_t)0x78);      // getByte 0
    ASSERT ((uint8_t)(global >>  8) == (uint8_t)0x56);      // getByte 1
    ASSERT ((uint8_t)(global >> 16) == (uint8_t)0x34);      // getByte 2
    ASSERT ((uint8_t)(global >> 24) == (uint8_t)0x12);      // getByte 3

    ASSERT ((uint16_t)(global >>  0) == (uint16_t)0x5678);  // getWord 0
    ASSERT ((uint16_t)(global >>  8) == (uint16_t)0x3456);  // getWord 1
    ASSERT ((uint16_t)(global >> 16) == (uint16_t)0x1234);  // getWord 2
}
