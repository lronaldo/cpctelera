/* bug-2761.c
   z80 code generation for ^, &, | failed when one operand is register A, while the other is a sfr.
 */

#include <testfwk.h>

#if defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_r2k) || defined(__SDCC_r2ka) || defined(__SDCC_r3ka) || defined(__SDCC_mcs51)
__sfr __at 0xB0 rtc_secl;

void fooX(void)
{
    unsigned char r = 0;
    do {
        r = rtc_secl;
    } while((r ^ rtc_secl) & 0x0F);
}

void fooA(void)
{
    unsigned char r = 0;
    do {
        r = rtc_secl;
    } while((r & rtc_secl) & 0x0F);
}

void fooO(void)
{
    unsigned char r = 0;
    do {
        r = rtc_secl;
    } while((r | rtc_secl) & 0x0F);
}
#endif

void testBug(void)
{
}

