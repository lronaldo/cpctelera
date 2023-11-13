/** Simple test for the _itoa and _uitoa.
  test: uitoa, itoa
  part: 1, 2, 3, 4, 5
*/

#include <testfwk.h>
#include <stdlib.h>
#include <string.h>

#if !defined(PORT_HOST) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
/* do not run tests for listed above platforms - out of memory */

#define TEST_{test} 1
#define ITOA(num,base,res) check_i(num, base, res)
#define UITOA(num,base,res) check_ui(num, base, res)

static char
check_i (int n, int b, const char *r)
{
  char buf[34];
  __itoa(n, buf, b);
  return !strcmp (buf, r);
}

static char
check_ui (int n, int b, const char *r)
{
  char buf[34];
  __uitoa(n, buf, b);
  return !strcmp (buf, r);
}

#endif

void test_itoa (void)
{
#ifdef TEST_itoa
#if {part} == 1
  ASSERT (ITOA (0, 2, "0"));
  ASSERT (ITOA (0, 8, "0"));
  ASSERT (ITOA (0, 10, "0"));
  ASSERT (ITOA (0, 16, "0"));
  ASSERT (ITOA (1, 2, "1"));
  ASSERT (ITOA (1, 8, "1"));
  ASSERT (ITOA (1, 10, "1"));
  ASSERT (ITOA (1, 16, "1"));
  ASSERT (ITOA (2, 2, "10"));
  ASSERT (ITOA (2, 8, "2"));
  ASSERT (ITOA (2, 10, "2"));
  ASSERT (ITOA (2, 16, "2"));
  ASSERT (ITOA (3, 2, "11"));
  ASSERT (ITOA (3, 8, "3"));
  ASSERT (ITOA (3, 10, "3"));
  ASSERT (ITOA (3, 16, "3"));
  ASSERT (ITOA (4, 2, "100"));
  ASSERT (ITOA (4, 8, "4"));
  ASSERT (ITOA (4, 10, "4"));
  ASSERT (ITOA (4, 16, "4"));
  ASSERT (ITOA (5, 2, "101"));
  ASSERT (ITOA (5, 8, "5"));
  ASSERT (ITOA (5, 10, "5"));
  ASSERT (ITOA (5, 16, "5"));
  ASSERT (ITOA (6, 2, "110"));
  ASSERT (ITOA (6, 8, "6"));
  ASSERT (ITOA (6, 10, "6"));
  ASSERT (ITOA (6, 16, "6"));
  ASSERT (ITOA (7, 2, "111"));
  ASSERT (ITOA (7, 8, "7"));
  ASSERT (ITOA (7, 10, "7"));
  ASSERT (ITOA (7, 16, "7"));
  ASSERT (ITOA (8, 2, "1000"));
  ASSERT (ITOA (8, 8, "10"));
  ASSERT (ITOA (8, 10, "8"));
  ASSERT (ITOA (8, 16, "8"));
  ASSERT (ITOA (9, 2, "1001"));
  ASSERT (ITOA (9, 8, "11"));
  ASSERT (ITOA (9, 10, "9"));
  ASSERT (ITOA (9, 16, "9"));
  ASSERT (ITOA (10, 2, "1010"));
  ASSERT (ITOA (10, 8, "12"));
  ASSERT (ITOA (10, 10, "10"));
  ASSERT (ITOA (10, 16, "A"));
  ASSERT (ITOA (11, 2, "1011"));
  ASSERT (ITOA (11, 8, "13"));
  ASSERT (ITOA (11, 10, "11"));
  ASSERT (ITOA (11, 16, "B"));
  ASSERT (ITOA (12, 2, "1100"));
  ASSERT (ITOA (12, 8, "14"));
  ASSERT (ITOA (12, 10, "12"));
#elif {part} == 2
  ASSERT (ITOA (12, 16, "C"));
  ASSERT (ITOA (13, 2, "1101"));
  ASSERT (ITOA (13, 8, "15"));
  ASSERT (ITOA (13, 10, "13"));
  ASSERT (ITOA (13, 16, "D"));
  ASSERT (ITOA (14, 2, "1110"));
  ASSERT (ITOA (14, 8, "16"));
  ASSERT (ITOA (14, 10, "14"));
  ASSERT (ITOA (14, 16, "E"));
  ASSERT (ITOA (15, 2, "1111"));
  ASSERT (ITOA (15, 8, "17"));
  ASSERT (ITOA (15, 10, "15"));
  ASSERT (ITOA (15, 16, "F"));
  ASSERT (ITOA (16, 2, "10000"));
  ASSERT (ITOA (16, 8, "20"));
  ASSERT (ITOA (16, 10, "16"));
  ASSERT (ITOA (16, 16, "10"));
  ASSERT (ITOA (-1, 2, "1111111111111111"));
  ASSERT (ITOA (-1, 8, "177777"));
  ASSERT (ITOA (-1, 10, "-1"));
  ASSERT (ITOA (-1, 16, "FFFF"));
  ASSERT (ITOA (-2, 2, "1111111111111110"));
  ASSERT (ITOA (-2, 8, "177776"));
  ASSERT (ITOA (-2, 10, "-2"));
  ASSERT (ITOA (-2, 16, "FFFE"));
  ASSERT (ITOA (-3, 2, "1111111111111101"));
  ASSERT (ITOA (-3, 8, "177775"));
  ASSERT (ITOA (-3, 10, "-3"));
  ASSERT (ITOA (-3, 16, "FFFD"));
  ASSERT (ITOA (-4, 2, "1111111111111100"));
  ASSERT (ITOA (-4, 8, "177774"));
  ASSERT (ITOA (-4, 10, "-4"));
  ASSERT (ITOA (-4, 16, "FFFC"));
  ASSERT (ITOA (-5, 2, "1111111111111011"));
  ASSERT (ITOA (-5, 8, "177773"));
  ASSERT (ITOA (-5, 10, "-5"));
  ASSERT (ITOA (-5, 16, "FFFB"));
  ASSERT (ITOA (-6, 2, "1111111111111010"));
  ASSERT (ITOA (-6, 8, "177772"));
  ASSERT (ITOA (-6, 10, "-6"));
  ASSERT (ITOA (-6, 16, "FFFA"));
  ASSERT (ITOA (-7, 2, "1111111111111001"));
  ASSERT (ITOA (-7, 8, "177771"));
  ASSERT (ITOA (-7, 10, "-7"));
  ASSERT (ITOA (-7, 16, "FFF9"));
  ASSERT (ITOA (-8, 2, "1111111111111000"));
  ASSERT (ITOA (-8, 8, "177770"));
  ASSERT (ITOA (-8, 10, "-8"));
  ASSERT (ITOA (-8, 16, "FFF8"));
  ASSERT (ITOA (-9, 2, "1111111111110111"));
#elif {part} == 3
  ASSERT (ITOA (-9, 8, "177767"));
  ASSERT (ITOA (-9, 10, "-9"));
  ASSERT (ITOA (-9, 16, "FFF7"));
  ASSERT (ITOA (-10, 2, "1111111111110110"));
  ASSERT (ITOA (-10, 8, "177766"));
  ASSERT (ITOA (-10, 10, "-10"));
  ASSERT (ITOA (-10, 16, "FFF6"));
  ASSERT (ITOA (-11, 2, "1111111111110101"));
  ASSERT (ITOA (-11, 8, "177765"));
  ASSERT (ITOA (-11, 10, "-11"));
  ASSERT (ITOA (-11, 16, "FFF5"));
  ASSERT (ITOA (-12, 2, "1111111111110100"));
  ASSERT (ITOA (-12, 8, "177764"));
  ASSERT (ITOA (-12, 10, "-12"));
  ASSERT (ITOA (-12, 16, "FFF4"));
  ASSERT (ITOA (-13, 2, "1111111111110011"));
  ASSERT (ITOA (-13, 8, "177763"));
  ASSERT (ITOA (-13, 10, "-13"));
  ASSERT (ITOA (-13, 16, "FFF3"));
  ASSERT (ITOA (-14, 2, "1111111111110010"));
  ASSERT (ITOA (-14, 8, "177762"));
  ASSERT (ITOA (-14, 10, "-14"));
  ASSERT (ITOA (-14, 16, "FFF2"));
  ASSERT (ITOA (-15, 2, "1111111111110001"));
  ASSERT (ITOA (-15, 8, "177761"));
  ASSERT (ITOA (-15, 10, "-15"));
  ASSERT (ITOA (-15, 16, "FFF1"));
  ASSERT (ITOA (-16, 2, "1111111111110000"));
  ASSERT (ITOA (-16, 8, "177760"));
  ASSERT (ITOA (-16, 10, "-16"));
  ASSERT (ITOA (-16, 16, "FFF0"));
  ASSERT (ITOA (127, 2, "1111111"));
  ASSERT (ITOA (127, 8, "177"));
  ASSERT (ITOA (127, 10, "127"));
  ASSERT (ITOA (127, 16, "7F"));
  ASSERT (ITOA (-127, 2, "1111111110000001"));
  ASSERT (ITOA (-127, 8, "177601"));
  ASSERT (ITOA (-127, 10, "-127"));
  ASSERT (ITOA (-127, 16, "FF81"));
  ASSERT (ITOA (128, 2, "10000000"));
  ASSERT (ITOA (128, 8, "200"));
  ASSERT (ITOA (128, 10, "128"));
  ASSERT (ITOA (128, 16, "80"));
  ASSERT (ITOA (-128, 2, "1111111110000000"));
  ASSERT (ITOA (-128, 8, "177600"));
  ASSERT (ITOA (-128, 10, "-128"));
  ASSERT (ITOA (-128, 16, "FF80"));
#elif {part} == 4
  ASSERT (ITOA (255, 2, "11111111"));
  ASSERT (ITOA (255, 8, "377"));
  ASSERT (ITOA (255, 10, "255"));
  ASSERT (ITOA (255, 16, "FF"));
  ASSERT (ITOA (-255, 2, "1111111100000001"));
  ASSERT (ITOA (-255, 8, "177401"));
  ASSERT (ITOA (-255, 10, "-255"));
  ASSERT (ITOA (-255, 16, "FF01"));
  ASSERT (ITOA (256, 2, "100000000"));
  ASSERT (ITOA (256, 8, "400"));
  ASSERT (ITOA (256, 10, "256"));
  ASSERT (ITOA (256, 16, "100"));
  ASSERT (ITOA (-256, 2, "1111111100000000"));
  ASSERT (ITOA (-256, 8, "177400"));
  ASSERT (ITOA (-256, 10, "-256"));
  ASSERT (ITOA (-256, 16, "FF00"));
  ASSERT (ITOA (0x1234, 2, "1001000110100"));
  ASSERT (ITOA (0x1234, 8, "11064"));
  ASSERT (ITOA (0x1234, 10, "4660"));
  ASSERT (ITOA (0x1234, 16, "1234"));
  ASSERT (ITOA (-0x1234, 2, "1110110111001100"));
  ASSERT (ITOA (-0x1234, 8, "166714"));
  ASSERT (ITOA (-0x1234, 10, "-4660"));
  ASSERT (ITOA (-0x1234, 16, "EDCC"));
  ASSERT (ITOA (0x5678, 2, "101011001111000"));
  ASSERT (ITOA (0x5678, 8, "53170"));
  ASSERT (ITOA (0x5678, 10, "22136"));
  ASSERT (ITOA (0x5678, 16, "5678"));
  ASSERT (ITOA (-0x5678, 2, "1010100110001000"));
  ASSERT (ITOA (-0x5678, 8, "124610"));
  ASSERT (ITOA (-0x5678, 10, "-22136"));
  ASSERT (ITOA (-0x5678, 16, "A988"));
  ASSERT (ITOA (012345, 2, "1010011100101"));
  ASSERT (ITOA (012345, 8, "12345"));
  ASSERT (ITOA (012345, 10, "5349"));
  ASSERT (ITOA (012345, 16, "14E5"));
  ASSERT (ITOA (-012345, 2, "1110101100011011"));
  ASSERT (ITOA (-012345, 8, "165433"));
  ASSERT (ITOA (-012345, 10, "-5349"));
  ASSERT (ITOA (-012345, 16, "EB1B"));
  ASSERT (ITOA (32767, 2, "111111111111111"));
  ASSERT (ITOA (32767, 8, "77777"));
  ASSERT (ITOA (32767, 10, "32767"));
  ASSERT (ITOA (32767, 16, "7FFF"));
  ASSERT (ITOA (-32767, 2, "1000000000000001"));
  ASSERT (ITOA (-32767, 8, "100001"));
  ASSERT (ITOA (-32767, 10, "-32767"));
  ASSERT (ITOA (-32767, 16, "8001"));
  ASSERT (ITOA (-32768, 2, "1000000000000000"));
  ASSERT (ITOA (-32768, 8, "100000"));
  ASSERT (ITOA (-32768, 10, "-32768"));
  ASSERT (ITOA (-32768, 16, "8000"));
#elif {part} == 5
  ASSERT (ITOA (1, 6, "1"));
  ASSERT (ITOA (12, 6, "20"));
  ASSERT (ITOA (123, 6, "323"));
#endif
#undef ITOA
#endif /* TEST_itoa */
}

void test_uitoa(void)
{
#ifdef TEST_uitoa
#if {part} == 1
  ASSERT (UITOA (0, 2, "0"));
  ASSERT (UITOA (0, 8, "0"));
  ASSERT (UITOA (0, 10, "0"));
  ASSERT (UITOA (0, 16, "0"));
  ASSERT (UITOA (1, 2, "1"));
  ASSERT (UITOA (1, 8, "1"));
  ASSERT (UITOA (1, 10, "1"));
  ASSERT (UITOA (1, 16, "1"));
  ASSERT (UITOA (2, 2, "10"));
  ASSERT (UITOA (2, 8, "2"));
  ASSERT (UITOA (2, 10, "2"));
  ASSERT (UITOA (2, 16, "2"));
  ASSERT (UITOA (3, 2, "11"));
  ASSERT (UITOA (3, 8, "3"));
  ASSERT (UITOA (3, 10, "3"));
  ASSERT (UITOA (3, 16, "3"));
  ASSERT (UITOA (4, 2, "100"));
  ASSERT (UITOA (4, 8, "4"));
  ASSERT (UITOA (4, 10, "4"));
  ASSERT (UITOA (4, 16, "4"));
  ASSERT (UITOA (5, 2, "101"));
  ASSERT (UITOA (5, 8, "5"));
  ASSERT (UITOA (5, 10, "5"));
  ASSERT (UITOA (5, 16, "5"));
  ASSERT (UITOA (6, 2, "110"));
  ASSERT (UITOA (6, 8, "6"));
  ASSERT (UITOA (6, 10, "6"));
#elif {part} == 2
  ASSERT (UITOA (6, 16, "6"));
  ASSERT (UITOA (7, 2, "111"));
  ASSERT (UITOA (7, 8, "7"));
  ASSERT (UITOA (7, 10, "7"));
  ASSERT (UITOA (7, 16, "7"));
  ASSERT (UITOA (8, 2, "1000"));
  ASSERT (UITOA (8, 8, "10"));
  ASSERT (UITOA (8, 10, "8"));
  ASSERT (UITOA (8, 16, "8"));
  ASSERT (UITOA (9, 2, "1001"));
  ASSERT (UITOA (9, 8, "11"));
  ASSERT (UITOA (9, 10, "9"));
  ASSERT (UITOA (9, 16, "9"));
  ASSERT (UITOA (10, 2, "1010"));
  ASSERT (UITOA (10, 8, "12"));
  ASSERT (UITOA (10, 10, "10"));
  ASSERT (UITOA (10, 16, "A"));
  ASSERT (UITOA (11, 2, "1011"));
  ASSERT (UITOA (11, 8, "13"));
  ASSERT (UITOA (11, 10, "11"));
  ASSERT (UITOA (11, 16, "B"));
  ASSERT (UITOA (12, 2, "1100"));
  ASSERT (UITOA (12, 8, "14"));
  ASSERT (UITOA (12, 10, "12"));
  ASSERT (UITOA (12, 16, "C"));
  ASSERT (UITOA (13, 2, "1101"));
  ASSERT (UITOA (13, 8, "15"));
#elif {part} == 3
  ASSERT (UITOA (13, 10, "13"));
  ASSERT (UITOA (13, 16, "D"));
  ASSERT (UITOA (14, 2, "1110"));
  ASSERT (UITOA (14, 8, "16"));
  ASSERT (UITOA (14, 10, "14"));
  ASSERT (UITOA (14, 16, "E"));
  ASSERT (UITOA (15, 2, "1111"));
  ASSERT (UITOA (15, 8, "17"));
  ASSERT (UITOA (15, 10, "15"));
  ASSERT (UITOA (15, 16, "F"));
  ASSERT (UITOA (16, 2, "10000"));
  ASSERT (UITOA (16, 8, "20"));
  ASSERT (UITOA (16, 10, "16"));
  ASSERT (UITOA (16, 16, "10"));
  ASSERT (UITOA (127, 2, "1111111"));
  ASSERT (UITOA (127, 8, "177"));
  ASSERT (UITOA (127, 10, "127"));
  ASSERT (UITOA (127, 16, "7F"));
  ASSERT (UITOA (255, 2, "11111111"));
  ASSERT (UITOA (255, 8, "377"));
  ASSERT (UITOA (255, 10, "255"));
  ASSERT (UITOA (255, 16, "FF"));
  ASSERT (UITOA (0x1234, 2, "1001000110100"));
  ASSERT (UITOA (0x1234, 8, "11064"));
  ASSERT (UITOA (0x1234, 10, "4660"));
  ASSERT (UITOA (0x1234, 16, "1234"));
  ASSERT (UITOA (0x5678, 2, "101011001111000"));
#elif {part} == 4
  ASSERT (UITOA (0x5678, 8, "53170"));
  ASSERT (UITOA (0x5678, 10, "22136"));
  ASSERT (UITOA (0x5678, 16, "5678"));
  ASSERT (UITOA (0x9abc, 2, "1001101010111100"));
  ASSERT (UITOA (0x9abc, 8, "115274"));
  ASSERT (UITOA (0x9abc, 10, "39612"));
  ASSERT (UITOA (0x9abc, 16, "9ABC"));
  ASSERT (UITOA (0xdef0, 2, "1101111011110000"));
  ASSERT (UITOA (0xdef0, 8, "157360"));
  ASSERT (UITOA (0xdef0, 10, "57072"));
  ASSERT (UITOA (0xdef0, 16, "DEF0"));
  ASSERT (UITOA (012345, 2, "1010011100101"));
  ASSERT (UITOA (012345, 8, "12345"));
  ASSERT (UITOA (012345, 10, "5349"));
  ASSERT (UITOA (012345, 16, "14E5"));
  ASSERT (UITOA (32767, 2, "111111111111111"));
  ASSERT (UITOA (32767, 8, "77777"));
  ASSERT (UITOA (32767, 10, "32767"));
  ASSERT (UITOA (32767, 16, "7FFF"));
  ASSERT (UITOA (32768, 2, "1000000000000000"));
  ASSERT (UITOA (32768, 8, "100000"));
  ASSERT (UITOA (32768, 10, "32768"));
  ASSERT (UITOA (32768, 16, "8000"));
  ASSERT (UITOA (65535, 2, "1111111111111111"));
  ASSERT (UITOA (65535, 8, "177777"));
  ASSERT (UITOA (65535, 10, "65535"));
  ASSERT (UITOA (65535, 16, "FFFF"));
#elif {part} == 5
  ASSERT (UITOA (1, 6, "1"));
  ASSERT (UITOA (12, 6, "20"));
  ASSERT (UITOA (123, 6, "323"));
#endif
#undef UITOA
#endif /* TEST_uitoa */
}

