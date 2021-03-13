/* bug 3065
   pdk backend generating invalid asm for some bitwise and on __sfr-
 */
#include <testfwk.h>

#if defined(__SDCC_pdk13) || defined(__SDCC_pdk14) || defined(__SDCC_pdk15)

__sfr __at(0x10) _pa;
#define PA        _pa

unsigned char b = 0;

void f(void)
{
    PA &= (unsigned char)~(1<<b);
}
#endif

void
testBug(void)
{
}

