/* bug-2368.c
   Wrong byte order in optimization of pointer assignment for stm8
 */

#include <testfwk.h>
#include <stdint.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if defined (__SDCC_stm8) || defined (__SDCC_z80) || defined (__SDCC_z180) || defined (__SDCC_r2k) || defined (__SDCC_r2ka) || defined (__SDCC_r3ka)

#define TEST_VAL 0x1234

#ifdef __SDCC_stm8 // data memory in lower half of 16-bit address space
#define loc ((volatile uint16_t *) 0x7fd)
#else // data memory in upper half of 16-bit address space
#define loc ((volatile uint16_t *) 0x87fd)
#endif

volatile uint16_t *p = loc;

void foo1 (void)
{
  *p = TEST_VAL;
}

void foo2 (void)
{
  *loc = TEST_VAL;
}

#endif

void testBug (void)
{
#if defined (__SDCC_stm8) || defined (__SDCC_z80) || defined (__SDCC_z180) || defined (__SDCC_r2k) || defined (__SDCC_r2ka) || defined (__SDCC_r3ka)
  volatile uint16_t *q = loc;

  foo1 ();
  ASSERT (*loc == TEST_VAL);
  ASSERT (*p == TEST_VAL);
  ASSERT (*q == TEST_VAL);

  foo2 ();
  ASSERT (*loc == TEST_VAL);
  ASSERT (*p == TEST_VAL);
  ASSERT (*q == TEST_VAL);

  *q = TEST_VAL;
  ASSERT (*loc == TEST_VAL);
  ASSERT (*p == TEST_VAL);
  ASSERT (*q == TEST_VAL);

#endif
}
