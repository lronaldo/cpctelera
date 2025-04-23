/* bug-2798.c

   z80 and pdk code generator do not support address of __sfr, but failed to warn
 */

#ifdef TEST1
#if !defined(__SDCC_stm8) && !defined(__SDCC_hc08) && !defined(__SDCC_s08) && !defined(__SDCC_mos6502) && !defined(__SDCC_mos65c02) && !defined(__SDCC_tlcs90) && !defined(__SDCC_sm83) 
__sfr __at 0x1234 x; /* IGNORE */

unsigned char *foo(void)
{
    return &x; /* WARNING(SDCC_z80|SDCC_z180|SDCC_r2k|SDCC_r2ka|SDCC_r3ka|SDCC_pdk13|SDCC_pdk14|SDCC_pdk15) */
}

#endif

void bar(void)
{
}

#endif

