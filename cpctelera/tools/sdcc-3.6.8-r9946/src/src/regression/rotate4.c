#include "gpsim_assert.h"
// rotate bytes left and right by a constant.

unsigned char failures=0;

unsigned int uint0 = 0;
unsigned int uint1 = 0;
unsigned char uchar0 = 0;
unsigned char uchar1 = 0;
unsigned char uchar2 = 0;

void
done()
{
  ASSERT(MANGLE(failures) == 0);
  PASSED();
}

void check(void)
{
  if(uchar0 != uchar1)
    failures++;
}

void rotate_left_1(void)
{

  uchar0 = (uchar0<<1) | (uchar0>>7);

  check();
}

void rotate_left_2(void)
{

  uchar0 = (uchar0<<2) | (uchar0>>6);

  check();
}


void rotate_left_3(void)
{

  uchar0 <<= 3;

  if(uchar0 != uchar1)
    failures++;
}

void rotate_left_4(void)
{

  uchar0 <<= 4;

  if(uchar0 != uchar1)
    failures++;
}

void rotate_left_5(void)
{

  uchar0 <<= 5;

  if(uchar0 != uchar1)
    failures++;
}

void rotate_left_6(void)
{

  uchar0 <<= 6;

  if(uchar0 != uchar1)
    failures++;
}

void rotate_left_7(void)
{

  uchar0 <<= 7;

  if(uchar0 != uchar1)
    failures++;
}

void rotate_right_1(void)
{

  uchar0 = (uchar0>>1) | (uchar0<<7);

  check();

}

void rotate_right_2(void)
{

  uchar0 = (uchar0>>2) | (uchar0<<6);

  check();
}

void rotate_right_3(void)
{

  uchar0 >>= 3;

  check();
}

void rotate_right_4(void)
{

  uchar0 >>= 4;

  check();
}

void rotate_right_5(void)
{

  uchar0 >>= 5;

  check();
}

void rotate_right_6(void)
{

  uchar0 >>= 6;

  check();
}

void rotate_right_7(void)
{

  uchar0 >>= 7;

  check();
}


void main(void)
{

  // call with both values zero
  rotate_left_1();

  uchar0 = 1;
  uchar1 = 2;

  rotate_left_1();

  uchar0 = 0x80;
  uchar1 = 1;

  rotate_left_1();

  uchar1 = 2;
  for(uchar2=0; uchar2<6; uchar2++) {
    rotate_left_1();
    uchar1 <<=1;
  }


  uchar0 = 1;
  uchar1 = 4;
  rotate_left_2();

  uchar0 = 1;
  uchar1 = 8;
  rotate_left_3();

  uchar0 = 1;
  uchar1 = 0x10;
  rotate_left_4();

  uchar0 = 1;
  uchar1 = 0x20;
  rotate_left_5();

  uchar0 = 1;
  uchar1 = 0x40;
  rotate_left_6();

  uchar0 = 1;
  uchar1 = 0x80;
  rotate_left_7();




  uchar0 = 2;
  uchar1 = 1;
  rotate_right_1();

  uchar0 = 1;
  uchar1 = 0x80;
  rotate_right_1();

  uchar0 = 4;
  uchar1 = 1;
  rotate_right_2();

  uchar0 = 8;
  rotate_right_3();

  uchar0 = 0x10;
  rotate_right_4();

  uchar0 = 0x20;
  rotate_right_5();

  uchar0 = 0x40;
  rotate_right_6();

  uchar0 = 0x80;
  rotate_right_7();

  done();
}
