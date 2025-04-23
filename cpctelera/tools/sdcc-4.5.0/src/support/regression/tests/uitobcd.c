/*
   uitobcd.c
*/

#include <testfwk.h>
#include <stdio.h>

extern void __uitobcd (unsigned int v, unsigned char r[3]);
#define CHECK(v)              \
    do {                      \
        static const unsigned char u[3] = { 0x##v % 0x100, (0x##v / 0x100) % 0x100, (0x##v / 0x10000) % 0x100 }; \
        unsigned char r[3] = { 0xff, 0xff, 0xff };   \
        __uitobcd (v, r);     \
        unsigned long bcd = r[2] * 0x10000 + r[1] * 0x100 + r[0]; \
        ASSERT ((#v,(r[0]==u[0]&&r[1]==u[1]&&r[2]==u[2]))); \
    } while (0)

void test__uitobcd (void)
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
  CHECK(31);
  CHECK(99);
  CHECK(100);
  CHECK(101);
  CHECK(127);
  CHECK(128);
  CHECK(998);
  CHECK(999);
  CHECK(1000);
  CHECK(1001);
  CHECK(1023);
  CHECK(1024);
  CHECK(9998);
  CHECK(9999);
  CHECK(10000);
  CHECK(10001);
  CHECK(32767);
  CHECK(32768);
  CHECK(65535);
#endif
}
