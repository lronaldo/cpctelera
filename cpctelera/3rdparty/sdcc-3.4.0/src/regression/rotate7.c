#include "gpsim_assert.h"
// Shift ints left and right

unsigned char failures=0;

signed int aint0 = 0;
signed int aint1 = 0;
signed char achar0 = 0;
signed char achar1 = 0;
signed char achar2 = 0;
signed char achar3 = 0;

void
done()
{
  ASSERT(MANGLE(failures) == 0);
  PASSED();
}

void shift_int_left_1(void)
{

  aint0 <<= 1;

}

void shift_int_left_2(void)
{

  aint0 <<= 2;

}

void shift_int_left_3(void)
{

  aint0 <<= 3;

}

void shift_int_left_4(void)
{

  aint0 <<= 4;

}

void shift_int_left_5(void)
{

  aint0 <<= 5;

}

void shift_int_left_6(void)
{

  aint0 <<= 6;

}

void shift_int_left_7(void)
{

  aint0 <<= 7;

}

void shift_int_left_8(void)
{

  aint0 <<= 8;

}

void shift_int_left_9(void)
{

  aint0 <<= 9;

}

void shift_int_left_10(void)
{

  aint0 <<= 10;

}

void shift_int_left_11(void)
{

  aint0 <<= 11;

}

void shift_int_left_12(void)
{

  aint0 <<= 12;

}

void shift_int_left_13(void)
{

  aint0 <<= 13;

}

void shift_int_left_14(void)
{

  aint0 <<= 14;

}

void shift_int_left_15(void)
{

  aint0 <<= 15;

}

/*****************************************************/
void shift_int_right_1(void)
{
  aint0 >>= 1;
}

void shift_int_right_2(void)
{
  aint0 >>= 2;
}

void shift_int_right_3(void)
{
  aint0 >>= 3;
}

void shift_int_right_4(void)
{
  aint0 >>= 4;
}

void shift_int_right_5(void)
{
  aint0 >>= 5;
}

void shift_int_right_6(void)
{
  aint0 >>= 6;
}

void shift_int_right_7(void)
{
  aint0 >>= 7;
}

void shift_int_right_8(void)
{
  aint0 >>= 8;
}

void shift_int_right_9(void)
{
  aint0 >>= 9;
}

void shift_int_right_10(void)
{
  aint0 >>= 10;
}

void shift_int_right_11(void)
{
  aint0 >>= 11;
}

void shift_int_right_12(void)
{
  aint0 >>= 12;
}

void shift_int_right_13(void)
{
  aint0 >>= 13;
}

void shift_int_right_14(void)
{
  aint0 >>= 14;
}

void shift_int_right_15(void)
{
  aint0 >>= 15;
}

/*****************************************************/
void main(void)
{
  //char i;

  aint0 = 0xabcd;

  shift_int_left_1();
  if(aint0 != 0x579a)
    failures++;

  aint0 = 0xabcd;

  shift_int_left_2();
  if(aint0 != 0xaf34)
    failures++;

  aint0 = 0xabcd;

  shift_int_left_3();
  if(aint0 != 0x5e68)
    failures++;

  aint0 = 0xabcd;

  shift_int_left_4();
  if(aint0 != 0xbcd0)
    failures++;

  aint0 = 0xabcd;

  shift_int_left_5();
  if(aint0 != 0x79a0)
    failures++;

  aint0 = 0xabcd;

  shift_int_left_6();
  if(aint0 != 0xf340)
    failures++;

  aint0 = 0xabcd;

  shift_int_left_7();
  if(aint0 != 0xe680)
    failures++;

  aint0 = 0xabcd;

  shift_int_left_8();
  if(aint0 != 0xcd00)
    failures++;

  aint0 = 0xabcd;

  shift_int_left_9();
  if(aint0 != 0x9a00)
    failures++;

  aint0 = 0xabcd;

  shift_int_left_10();
  if(aint0 != 0x3400)
    failures++;

  aint0 = 0xabcd;

  shift_int_left_11();
  if(aint0 != 0x6800)
    failures++;

  aint0 = 0xabcd;

  shift_int_left_12();
  if(aint0 != 0xd000)
    failures++;

  aint0 = 0xabcd;

  shift_int_left_13();
  if(aint0 != 0xa000)
    failures++;

  aint0 = 0xabcd;

  shift_int_left_14();
  if(aint0 != 0x4000)
    failures++;

  aint0 = 0xabcd;

  shift_int_left_15();
  if(aint0 != 0x8000)
    failures++;

  /***********************/
  aint0 = 0xabcd;

  shift_int_right_1();
  if(aint0 != 0xd5e6)
    failures++;

  aint0 = 0xabcd;

  shift_int_right_2();
  if(aint0 != 0xeaf3)
    failures++;

  aint0 = 0xabcd;

  shift_int_right_3();
  if(aint0 != 0xf579)
    failures++;

  aint0 = 0xabcd;

  shift_int_right_4();
  if(aint0 != 0xfabc)
    failures++;

  aint0 = 0xabcd;

  shift_int_right_5();
  if(aint0 != 0xfd5e)
    failures++;

  aint0 = 0xabcd;

  shift_int_right_6();
  if(aint0 != 0xfeaf)
    failures++;

  aint0 = 0xabcd;

  shift_int_right_7();
  if(aint0 != 0xff57)
    failures++;

  aint0 = 0xabcd;

  shift_int_right_8();
  if(aint0 != 0xffab)
    failures++;

  aint0 = 0xabcd;

  shift_int_right_9();
  if(aint0 != 0xffd5)
    failures++;

  aint0 = 0xabcd;

  shift_int_right_10();
  if(aint0 != 0xffea)
    failures++;

  aint0 = 0xabcd;

  shift_int_right_11();
  if(aint0 != 0xfff5)
    failures++;

  aint0 = 0xabcd;

  shift_int_right_12();
  if(aint0 != 0xfffa)
    failures++;

  aint0 = 0xabcd;

  shift_int_right_13();
  if(aint0 != 0xfffd)
    failures++;

  aint0 = 0xabcd;

  shift_int_right_14();
  if(aint0 != 0xfffe)
    failures++;

  aint0 = 0xabcd;

  shift_int_right_15();
  if(aint0 != -1)
    failures++;

  done();
}
