/*
    bug-?.c
*/

#include <testfwk.h>

static const char *expect;

int pc(int c)
{
  ASSERT (c == *expect++);
}

void printhex_le(unsigned char x)
{
  unsigned char n = x>>4;
  if( n>9 ) pc('A'-10+n); else pc('0'+n); // Bug for n == 'a'. Works when >9 is replaced by >=10
  n = x&0xF;
  if( n>9 ) pc('A'-10+n); else pc('0'+n); // Bug for n == 'a'. Works when >9 is replaced by >=10
}

void printhex_leq(unsigned char x)
{
  unsigned char n = x>>4;
  if( n>=10 ) pc('A'-10+n); else pc('0'+n);
  n = x&0xF;
  if( n>=10 ) pc('A'-10+n); else pc('0'+n);
}

void testBug(void)
{
  expect = "01";
  printhex_le (0x01);
  expect = "23";
  printhex_le (0x23);
  expect = "45";
  printhex_le (0x45);
  expect = "56";
  printhex_le (0x56);
  expect = "78";
  printhex_le (0x78);
  expect = "9A";
  printhex_le (0x9a);
  expect = "BC";
  printhex_le (0xbc);
  expect = "DE";
  printhex_le (0xde);

  expect = "10";
  printhex_le (0x10);
  expect = "32";
  printhex_le (0x32);
  expect = "54";
  printhex_le (0x54);
  expect = "65";
  printhex_le (0x65);
  expect = "87";
  printhex_le (0x87);
  expect = "A9";
  printhex_le (0xa9);
  expect = "CB";
  printhex_le (0xcb);
  expect = "ED";
  printhex_le (0xed);

  expect = "01";
  printhex_leq (0x01);
  expect = "23";
  printhex_leq (0x23);
  expect = "45";
  printhex_leq (0x45);
  expect = "56";
  printhex_leq (0x56);
  expect = "78";
  printhex_leq (0x78);
  expect = "9A";
  printhex_leq (0x9a);
  expect = "BC";
  printhex_leq (0xbc);
  expect = "DE";
  printhex_leq (0xde);

  expect = "10";
  printhex_leq (0x10);
  expect = "32";
  printhex_leq (0x32);
  expect = "54";
  printhex_leq (0x54);
  expect = "65";
  printhex_leq (0x65);
  expect = "87";
  printhex_leq (0x87);
  expect = "A9";
  printhex_leq (0xa9);
  expect = "CB";
  printhex_leq (0xcb);
  expect = "ED";
  printhex_leq (0xed);
}

