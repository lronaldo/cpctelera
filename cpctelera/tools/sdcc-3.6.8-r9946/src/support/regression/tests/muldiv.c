/** Simple test for the mul/div/mod operations.

    type: int, char, short, long
    storage: static,
    attr: volatile,
*/
#include <testfwk.h>

void
testUnsignedModDiv(void)
{
    {attr} {storage} unsigned {type} i;
    unsigned {type} result;

    i = 100;

    result = i/3;
    ASSERT(result == 33);

    result = i/12;
    ASSERT(result == 8);

    result = i%7;
    ASSERT(result == 2);

    result = i%34;
    ASSERT(result == 32);
}

void
testUnsignedMul(void)
{
    {attr} {storage} unsigned {type} i;
    unsigned {type} result;

    i = 37;

    LOG(("i*3 == 111 = %u\n", (int)(i*3)));
    result = i*3;
    ASSERT(result == 111);

    result = i*12;
    ASSERT(result == ((unsigned {type})444));
}

void
testMul(void)
{
    {attr} {storage} signed {type} i;
    signed {type} result;

    i = 5;

    LOG(("i*5 == 25 = %d\n", (int)(i*5)));
    result = i*5;
    ASSERT(result == 25);
    LOG(("i*-4 == -20 = %d\n", (int)(i*-4)));
    ASSERT(i*-4 == -20);
    i = -10;
#ifndef __SDCC_pic16
    LOG(("i*12 == -120 = %d\n", (int)(i*12)));
    ASSERT(i*12 == -120);
    LOG(("i*-3 == 30 = %d\n", (int)(i*-3)));
    ASSERT(i*-3 == 30);
#endif
}

void mark(void)
{
}

void
testDiv(void)
{
    {attr} {storage} signed {type} i;

    i = 100;
    LOG(("i/5 == 20 = %d\n", (int)i/5));
    ASSERT(i/5 == 20);
    LOG(("i/-4 == -25 = %d\n", (int)i/-4));
    mark();
    ASSERT(i/-4 == -25);

    i = -50;
    LOG(("i/25 == -2 = %d\n", (int)i/25));
    ASSERT(i/25 == -2);
    LOG(("i/-12 == 4 = %d\n", (int)i/-12));
    ASSERT(i/-12 == 4);
    //power of 2
    ASSERT(i/4 == -12);
}

void 
test16to32(void)
{
   {attr} {storage} int i, j;
   {attr} {storage} unsigned int ui, uj;

   i = 42;
   j = 42;
   ASSERT((long)i * (long)j == 42l * 42l);
   i = -i;
   ASSERT((long)i * (long)j == -42l * 42l);
   j = -j;
   ASSERT((long)i * (long)j == -42l * -42l);
   i = 2342;
   j = 4223;
   ASSERT((unsigned long)i * (unsigned long)j == 2342ul * 4223ul);
   ASSERT((long)i * (long)j == 2342l * 4223l);
   j = -j;
   ASSERT((long)i * (long)j == 2342l * -4223l);
   i = -i;
   ASSERT((long)i * (long)j == -2342l * -4223l);

   ui = 42;
   uj = 42;
   ASSERT((unsigned long)ui * (unsigned long)uj == 42ul * 42ul);
   ui = 2342;
   uj = 4223;
   ASSERT((unsigned long)ui * (unsigned long)uj == 2342ul * 4223ul);
   ui = 0xffff;
   uj = 0x8000;
   ASSERT((unsigned long)ui * (unsigned long)uj == 0xfffful * 0x8000ul);
}

void
testMod(void)
{
    {attr} {storage} signed {type} i;

    // Disabled the LOG functions due to a bug in sdcc involving
    // vaargs.
    i = 100;
    //    LOG(("i%%17 == 15 = %u\n", (int)(i%9)));
    ASSERT(i%17 == 15);

    //    LOG(("i%%-7 == 2 = %u\n", (int)i%-7));
    ASSERT(i%-7 == 2);
    //power of 2
    ASSERT(i%-8 == 4);

    i = -49;
    //    LOG(("i%%3 == -1 = %u\n", (int)i%3));
    ASSERT(i%3 == -1);
    //    LOG(("i%%-5 == -4 = %u\n", (int)i%-5));
    ASSERT(i%-5 == -4);
    //power of 2
    ASSERT(i%4 == -1);
}
