/** array test
    type: char, int
    storage: __xdata, __code,
*/
#include <testfwk.h>

#define TC(x) (0x10+(x))
#define TI(x) (0x1020+(x) + 0x100*(x))
#define TL(x) (0x10203040+(x))

const {storage} unsigned char array_const_char[4] = {TC(0), TC(1), TC(2), TC(3)};
const {storage} unsigned int  array_const_int [4] = {TI(0), TI(1), TI(2), TI(3)};
const {storage} unsigned long array_const_long[4] = {TL(0), TL(1), TL(2), TL(3)};

unsigned char array_char[4] = {TC(0), TC(1), TC(2), TC(3)};
unsigned int  array_int [4] = {TI(0), TI(1), TI(2), TI(3)};
unsigned long array_long[4] = {TL(0), TL(1), TL(2), TL(3)};

volatile unsigned {type} idx;
volatile unsigned {type} idx2;

void
testArrayAccess(void)
{
  idx = 2;

  ASSERT(array_const_char[idx] == TC(2));
  ASSERT(array_const_int [idx] == TI(2));
  ASSERT(array_const_long[idx] == TL(2));

  ASSERT(array_const_char[2] == TC(2));
  ASSERT(array_const_int [2] == TI(2));
  ASSERT(array_const_long[2] == TL(2));

  ASSERT(array_char[idx] == TC(2));
  ASSERT(array_int [idx] == TI(2));
  ASSERT(array_long[idx] == TL(2));

  ASSERT(array_char[2] == TC(2));
  ASSERT(array_int [2] == TI(2));
  ASSERT(array_long[2] == TL(2));

  idx = 3;
  idx2 = 1;

  array_char[idx2] = array_const_char[idx] | 0x80;
  array_int [idx2] = array_const_int [idx] | 0x8080;
  array_long[idx2] = array_const_long[idx] | 0x80808080;

  ASSERT(array_char[idx2] == (TC(3) | 0x80));
  ASSERT(array_int [idx2] == (TI(3) | 0x8080));
  ASSERT(array_long[idx2] == (TL(3) | 0x80808080));
}
