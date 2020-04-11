/*
  bug3475656.c

  Digits in output were reversed (e.g. 192 was written as "291").
*/

#include <testfwk.h>

#ifdef __SDCC
#pragma disable_warning 85
#endif

#define UCHAR   unsigned char
#define USHORT  unsigned short
#define PCHAR   unsigned char *

UCHAR mod_16x8(USHORT s16, UCHAR i8)
{
    return(s16 % i8);
}

USHORT div_16x8(USHORT s16, UCHAR i8)
{
    return(s16 / i8);
}

#if 1
void itoa(USHORT sVal, PCHAR pBuf, UCHAR iRadix)
{
    PCHAR p;            // pointer to traverse string
    PCHAR pFirstDigit;  // pointer to first digit
    UCHAR iTmp;         // temp UCHAR

    p = pBuf;
    pFirstDigit = p;    // save pointer to first digit

    do 
    {
        iTmp = mod_16x8(sVal, iRadix);
        sVal = div_16x8(sVal, iRadix);

        // convert to ascii and store
        *p++ = (iTmp > 9) ? iTmp - 10 + 'A' : iTmp + '0';  // a letter or a digit
    } while (sVal > 0);

    // We now have the digit of the number in the buffer, but in reverse order.  Thus we reverse them now.

    *p-- = '\0';               // terminate string; p points to last digit

    do
    {
        iTmp = *p;
        *p = *pFirstDigit;
        *pFirstDigit = iTmp;   // swap *p and *pFirstDigit
        --p;
        ++pFirstDigit;         // advance to next two digits
    } while (pFirstDigit < p); // repeat until halfway
}
#endif

void testBug(void)
{
    unsigned char c[8];
    unsigned short i;

    i = 192;
#if 1
    itoa(i, c, 10);
    ASSERT(c[0] == '1');
    ASSERT(c[1] == '9');
    ASSERT(c[2] == '2');
#endif
}
