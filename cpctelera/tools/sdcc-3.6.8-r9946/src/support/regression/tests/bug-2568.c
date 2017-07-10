/*
   bug-2568.c
*/

#include <testfwk.h>

#include <stdint.h>

#if defined(__SDCC_pic16)
#define ADDR 0x0200
#elif defined(__SDCC_pic14)
#define ADDR 0x0100
#elif defined(__SDCC_stm8)
#define ADDR 0x1000
#else
#define ADDR 0xca00
#endif

#if defined(__SDCC) && !defined(__SDCC_mcs51) && !defined(__SDCC_ds390)
uint16_t __at(ADDR) a[6] = {0, 1, 2, 3, 4, 5};
#endif

void testBug(void)
{
#if defined(__SDCC) && !defined(__SDCC_mcs51) && !defined(__SDCC_ds390)
    volatile uint8_t v;
    uint16_t eeprom1, eeprom2;

    eeprom1 = ((uint16_t *)ADDR)[4];
    v = 4; eeprom2 = ((uint16_t *)ADDR)[v];

    ASSERT(eeprom1 == eeprom2);
#endif
}

