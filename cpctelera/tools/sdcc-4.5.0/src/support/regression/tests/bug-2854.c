/* bug-2854.c
   A bug in handling of 8-bit parameters to z88dk_fastcall. Also bug #2852.
 */

#include <testfwk.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#pragma disable_warning 85
#pragma disable_warning 127

#if !defined( __SDCC_pdk14) && !defined( __SDCC_pdk15) && !(defined( __SDCC_mcs51) && (defined(__SDCC_MODEL_SMALL) || defined(__SDCC_MODEL_MEDIUM))) // Lack of memory

void lock_give_fastcall(uint8_t * mutex)
#ifdef __SDCC_z80
__preserves_regs(b,c,d,e,iyh,iyl) __z88dk_fastcall
#endif
{
}
#define lock_give(a) lock_give_fastcall(a)

uint8_t bank_get_abs_fastcall(int8_t bankRel)
#ifdef __SDCC_z80
__preserves_regs(b,c,d,e,h,iyh,iyl) __z88dk_fastcall
#endif
{
  ASSERT (bankRel = 23);
  return 0;
}
#define bank_get_abs(a) bank_get_abs_fastcall(a)

static void *memcpy_far(void *str1,int8_t bank1,const void *str2,const int8_t bank2,size_t n)
#ifdef __SDCC_z80
__preserves_regs(iyh,iyl)
#endif
{
}

uint8_t bankLockBase[16];

static void *buffer;

int8_t ya_mvb(const char **args)
{
    if ( (args[2] != NULL) && (bank_get_abs((int8_t)atoi(args[1])) != 0) && (bank_get_abs((int8_t)atoi(args[2])) != 0) )
    {
        memcpy_far((void *)0x0000, (int8_t)atoi(args[2]), (void *)0x0000, (int8_t)atoi(args[1]), (0xF000));
        bankLockBase[ bank_get_abs((int8_t)atoi(args[2])) ] = bankLockBase[ bank_get_abs((int8_t)atoi(args[1])) ];
    }
    return 1;
}

int8_t ya_loadb(const char **args)
{
    uint8_t * dest;
    uint32_t p1;
    uint16_t s1 = 0;

    if (args[1] == NULL || args[2] == NULL) {
    } else {
        if (args[3] == NULL) {
            dest = (uint8_t *)0x0100;
        } else {
            dest = (uint8_t *)strtoul(args[3], NULL, 16);
        }
        p1 = 0;
        while ((uint16_t)dest < (0xF000)) {
            if (s1 == 0) break;

            if (s1 > (0xF000) - (uint16_t)dest) {
                s1 = (0xF000) - (uint16_t)dest;
            }
            memcpy_far((void *)dest, (int8_t)atoi(args[2]), buffer, 0, s1);
            dest += s1;
            p1 += s1;
        }

        lock_give( &bankLockBase[ bank_get_abs((int8_t)atoi(args[2])) ] );
    }
    return 1;
}
#endif

void
testBug(void)
{
#if !defined( __SDCC_pdk14) && !defined( __SDCC_pdk15) && !(defined(__SDCC_mcs51) && (defined(__SDCC_MODEL_SMALL) || defined(__SDCC_MODEL_MEDIUM))) // Lack of memory
  const char *args[] = {"", "", "23", 0};
  char b;
  buffer = &b;

  ya_mvb(args);

  ya_loadb(args);
#endif
}

