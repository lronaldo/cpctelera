/* bug-3773.c

   Missing diagnostic on some out-of-range __data and __idata adresses for mcs51.
 */

#ifdef TEST1
#ifdef __SDCC_mcs51
__data __at (0x7e) unsigned int mcs51_data16_nowarn;
__idata __at (0xfe) unsigned int mcs51_idata16_nowarn;
__sfr __at (0xfe) mcs51_sfr8_nowarn;
__data __at (0x80) unsigned int mcs51_data8_warn;      /* WARNING(SDCC_mcs51) */
__idata __at (0x100) unsigned int mcs51_idata8_warn;   /* WARNING(SDCC_mcs51) */
__data __at (0x7f) unsigned int mcs51_data16_warn;     /* WARNING(SDCC_mcs51) */
__idata __at (0xff) unsigned int mcs51_idata16_warn;   /* WARNING(SDCC_mcs51) */
__sfr __at (0x7f) mcs51_sfr8_warn;                     /* WARNING(SDCC_mcs51) */
#endif
#if defined(__SDCC_mcs51) || defined(__SDCC_hc08) || defined(__SDCC_s08) || defined(__SDCC_mos6502)
__data __at (0xfe) unsigned int hc08_data16_nowarn;    /* WARNING(SDCC_mcs51) */
__data __at (0x100) unsigned int hc08_data8_warn;      /* WARNING(SDCC_mcs51|SDCC_hc08|SDCC_s08|SDCC_mos6502) */
__data __at (0xff) unsigned int hc08_data16_warn;      /* WARNING(SDCC_mcs51|SDCC_hc08|SDCC_s08|SDCC_mos6502) */
#endif
#if defined(__SDCC_mcs51) || defined(__SDCC_sm83) || defined(__SDCC_z80)
__sfr __at (0xff00) sm83_sfr8_nowarn;                  /* WARNING(SDCC_mcs51|SDCC_z80) */
__sfr __at (0x7f) sm83_sfr8_warn;                      /* WARNING(SDCC_mcs51|SDCC_sm83) */
#endif
#if defined(__SDCC_mcs51) ||  defined(__SDCC_pdk13) || defined(__SDCC_pdk14) || defined(__SDCC_pdk15) || defined(__SDCC_pdk16) || defined(__SDCC_z80)
__sfr __at (0x1f) pdk13_sfr8_nowarn;                   /* WARNING(SDCC_mcs51) */
__sfr __at (0x20) pdk13_sfr8_warn;                     /* WARNING(SDCC_mcs51|SDCC_pdk13) */
__sfr __at (0x3f) pdk14_sfr8_nowarn;                   /* WARNING(SDCC_mcs51|SDCC_pdk13) */
__sfr __at (0x40) pdk14_sfr8_warn;                     /* WARNING(SDCC_mcs51|SDCC_pdk13|SDCC_pdk14) */
__sfr __at (0x7f) pdk15_sfr8_nowarn;                   /* WARNING(SDCC_mcs51|SDCC_pdk13|SDCC_pdk14) */
__sfr __at (0x80) pdk15_sfr8_warn;                     /* WARNING(SDCC_pdk13|SDCC_pdk14|SDCC_pdk15) */
#endif

unsigned char c; // Avoid empty translation unit for other ports.
#endif

