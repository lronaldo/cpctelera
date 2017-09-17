/** Tests covering the shift operators.

    sign: signed, unsigned
    type: char, int, long
    storage: static,
    attr: volatile

    vals: 3

    pending - 1792, 851968, 1560281088, -3, -1792, -851968, -1560000000
*/
#include <testfwk.h>

void
test1ShiftClasses(void)
{
    {attr} {storage} {sign} {type} i, result;

    i = 30;
    ASSERT(i>>3 == 3);
    ASSERT(i<<2 == 120);

    result = i;
    result >>= 2;
    ASSERT(result == 7);

    result = i;
    result <<= 2;
    ASSERT(result == 120);
}

/* This tests for implementation-defined behaviour (right-shifting negative values).
   For sdcc the implementation defined behaviour is that right shift for arithmetic
   types is arithmetic. */
void
test2ShiftRight(void)
{
    {attr} {storage} signed {type} i, result;

    i = -120;
    ASSERT(i>>1 == -60);
    ASSERT(i>>2 == -30);
    ASSERT(i>>3 == -15);
    ASSERT(i>>4 == -8);
    ASSERT(i>>5 == -4);
    ASSERT(i>>6 == -2);
    ASSERT(i>>7 == -1);
    ASSERT(i>>8 == -1);
    result = i;
    result >>= 3;
    ASSERT(result == -15);
}

void
test3ShiftByteMultiples(void)
{
    {attr} {storage} {type} i;

    i = ({type}){vals};
    ASSERT(i>>8  == ({type})({vals} >> 8));
    ASSERT(i>>16 == ({type})({vals} >> 16));
    ASSERT(i>>24 == ({type})({vals} >> 24));

    i = ({type}){vals};
    ASSERT( ({type})(i<<8)  ==  ({type})({vals} << 8));;
    ASSERT((({type}) i<<16) == (({type}) {vals} << 16));
    ASSERT((({type}) i<<24) == (({type}) {vals} << 24));
}

void
test4ShiftOne(void)
{
    {attr} {storage} {sign} {type} i;
    {sign} {type} result;

    i = ({type}){vals};

    result = i >> 1;
    ASSERT(result == ({type})(({type}){vals} >> 1));

    result = i;
    result >>= 1;
    ASSERT(result == ({type})(({type}){vals} >> 1));

    result = i << 1;
    ASSERT(result == ({type})(({type}){vals} << 1));

    result = i;
    result <<= 1;
    ASSERT(result == ({type})(({type}){vals} << 1));
}

static {type} ShiftLeftByParam ({type} count)
{
    {attr} {storage} {type} i;
    i = ({type}){vals};
    return (i << count);
}

static {type} ShiftRightByParam ({type} count)
{
    {attr} {storage} {type} i;
    i = ({type}){vals};
    return (i >> count);
}

void
testShiftByParam(void)
{
    ASSERT(ShiftLeftByParam(2)  == ({type})({vals} << 2));
    ASSERT(ShiftRightByParam(2) == ({type})({vals} >> 2));
}

