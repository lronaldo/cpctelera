/*
   ultobcd.c
*/

#include <testfwk.h>
#include <stdio.h>

extern void __ultobcd (unsigned long v, unsigned char r[5]);
#define CHECK(v)              \
    do {                      \
        static const unsigned char u[5] = { 0x##v % 0x100, (0x##v / 0x100) % 0x100, (0x##v / 0x10000) % 0x100, (0x##v / 0x1000000) % 0x100, (0x##v / 0x100000000) % 0x100 }; \
        unsigned char r[5] = { 0xff, 0xff, 0xff, 0xff, 0xff };   \
        __ultobcd (v, r);     \
        unsigned long bcd = r[4] * 0x100000000 + r[3] * 0x1000000 + r[2] * 0x10000 + r[1] * 0x100 + r[0]; \
        ASSERT ((#v,(r[0]==u[0]&&r[1]==u[1]&&r[2]==u[2]&&r[3]==u[3]&&r[4]==u[4]))); \
    } while (0)

void test__ultobcd (void)
{
#if defined(__SDCC_z80)
  CHECK(0);
  CHECK(1);
  CHECK(2);
  CHECK(9);
  CHECK(10);
  CHECK(11);
  CHECK(15);
  CHECK(16);
  CHECK(100000);
  CHECK(345678);
  CHECK(2345678);
  CHECK(12345678);
  CHECK(16777215);   //0x00ffffff
  CHECK(234567890);
  CHECK(2147483647); //0x7fffffff
  CHECK(2147483648); //0x80000000
  CHECK(3000000000);
  CHECK(3999999999);
  CHECK(4000000000);
  CHECK(4294967295); //0xffffffff
#endif
}
