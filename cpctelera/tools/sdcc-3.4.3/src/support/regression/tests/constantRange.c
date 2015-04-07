/** constant range tests.

*/

#include <testfwk.h>

#ifdef __SDCC
#pragma std_sdcc99
#endif

#include <stdbool.h>
#ifdef __sun__
#include <inttypes.h>
#else
#include <stdint.h>
#endif

#if defined (__GNUC__) && defined (__alpha__) && (__GNUC__ < 3)
/* bool should be casted to int in order to pass the test. Is this a gcc bug? */
#define INT_CAST  (int)
#else
#define INT_CAST
#endif

 int8_t s8;
uint8_t u8;

 int16_t s16;
uint16_t u16;

 int32_t s32;
uint32_t u32;

void
testConstantRange (void)
{
  ASSERT (! (INT8_MIN - 1 == s8));
  ASSERT (! (INT8_MAX + 1 == s8));
  ASSERT (  (INT8_MIN - 1 != s8));
  ASSERT (  (INT8_MAX + 1 != s8));
  ASSERT (  (INT8_MIN - 1 <  s8));
  ASSERT (! (INT8_MAX     <  s8));
  ASSERT (  (INT8_MIN     <= s8));
  ASSERT (! (INT8_MAX     <= s8));
  ASSERT (! (INT8_MIN     >  s8));
  ASSERT (  (INT8_MAX + 1 >  s8));
  ASSERT (! (INT8_MIN - 1 >= s8));
  ASSERT (  (INT8_MAX     >= s8));

  ASSERT (! (        0 - 1 == u8));
  ASSERT (! (UINT8_MAX + 1 == u8));
  ASSERT (  (        0 - 1 != u8));
  ASSERT (  (UINT8_MAX + 1 != u8));
  ASSERT (  (        0 - 1 <  u8));
  ASSERT (! (UINT8_MAX     <  u8));
  ASSERT (  (        0     <= u8));
  ASSERT (! (UINT8_MAX + 1 <= u8));
  ASSERT (! (        0     >  u8));
  ASSERT (  (UINT8_MAX + 1 >  u8));
  ASSERT (! (        0 - 1 >= u8));
  ASSERT (  (UINT8_MAX     >= u8));

  /* force extension to long to avoid int (16 bit) overflow */
  ASSERT (! (INT16_MIN - 1L == s16));
  ASSERT (! (INT16_MAX + 1L == s16));
  ASSERT (  (INT16_MIN - 1L != s16));
  ASSERT (  (INT16_MAX + 1L != s16));
  ASSERT (  (INT16_MIN - 1L <  s16));
  ASSERT (! (INT16_MAX      <  s16));
  ASSERT (  (INT16_MIN      <= s16));
  ASSERT (! (INT16_MAX      <= s16));
  ASSERT (! (INT16_MIN      >  s16));
  ASSERT (  (INT16_MAX + 1L >  s16));
  ASSERT (! (INT16_MIN - 1L >= s16));
  ASSERT (  (INT16_MAX      >= s16));

  ASSERT (! (         0 - 1  == u16));
  ASSERT (! (UINT16_MAX + 1L == u16));
  ASSERT (  (         0 - 1  != u16));
  ASSERT (  (UINT16_MAX + 1L != u16));
  ASSERT (! (UINT16_MAX      <  u16));
  ASSERT (  (         0      <= u16));
  ASSERT (! (UINT16_MAX + 1L <= u16));
  ASSERT (! (         0      >  u16));
  ASSERT (  (UINT16_MAX + 1L >  u16));
  ASSERT (  (UINT16_MAX      >= u16));

#if defined(PORT_HOST)
/* on 32bit host: -1 is presented as 32 bit int, 16 bit unsigned short is promoted to 32 bit int */
  ASSERT (  (         0 - 1  <  u16)); /* -1 > 0 */
  ASSERT (! (         0 - 1  >= u16)); /* !(-1 <= 0) */
#else
/* on 16bit sdcc: int (-1) is promoted to unsigned int (0xffff) */
  ASSERT (  (         0 - 1  >  u16)); /* 0xffff > 0 */
  ASSERT (! (         0 - 1  <= u16)); /* !(0xffff <= 0) */
#endif

   /* sdcc can't hold a number (INT32_MIN - 1) or (INT32_MAX + 1),
      there's no 'double' or 'long long' */
/* ASSERT (! (INT32_MIN - 1 == s32)); */
/* ASSERT (! (INT32_MAX + 1 == s32)); */
/* ASSERT (  (INT32_MIN - 1 != s32)); */
/* ASSERT (  (INT32_MAX + 1 != s32)); */
/* ASSERT (  (INT32_MIN - 1 <  s32)); */
   ASSERT (! (INT32_MAX     <  s32));
   ASSERT (  (INT32_MIN     <= s32));
   ASSERT (! (INT32_MAX     <= s32));
   ASSERT (! (INT32_MIN     >  s32));
/* ASSERT (  (INT32_MAX + 1 >  s32)); */
/* ASSERT (! (INT32_MIN - 1 >= s32)); */
   ASSERT (  (INT32_MAX     >= s32));

   ASSERT (! (         0 - 1 == u32));
/* ASSERT (! (UINT32_MAX + 1 == u32)); */
   ASSERT (  (         0 - 1 != u32));
/* ASSERT (  (UINT32_MAX + 1 != u32)); */
   ASSERT (  (         0 - 1 >  u32)); /* 0xffffffff > 0 */
   ASSERT (! (UINT32_MAX     <  u32));
   ASSERT (  (         0     <= u32));
/* ASSERT (! (UINT32_MAX + 1 <= u32)); */
   ASSERT (! (         0     >  u32));
/* ASSERT (  (UINT32_MAX + 1 >  u32)); */
   ASSERT (! (         0 - 1 <= u32)); /* !(0xffffffff <= 0) */
   ASSERT (  (UINT32_MAX     >= u32));
}

void
testFoo1(void)
{
#ifdef __bool_true_false_are_defined

#if !defined(__SDCC_WEIRD_BOOL)
   volatile bool sb, ub;
#else
   volatile   signed bool sb;
   volatile unsigned bool ub;
#endif

  sb = 0;
  ub = 0;

  ASSERT (! (-1 == sb));
  ASSERT (  ( 0 == sb));
  ASSERT (! ( 1 == sb));

  ASSERT (  (-1 != sb));
  ASSERT (! ( 0 != sb));
  ASSERT (  ( 1 != sb));

  ASSERT (  (-1 <  INT_CAST sb));
  ASSERT (! ( 0 <  sb));

  ASSERT (  ( 0 <= sb));
  ASSERT (! ( 1 <= sb));

  ASSERT (! ( 0 >  sb));
  ASSERT (  ( 1 >  sb));

  ASSERT (! (-1 >= INT_CAST sb));
  ASSERT (  ( 0 >= sb));


  ASSERT (! (-1 == ub));
  ASSERT (  ( 0 == ub));
  ASSERT (! ( 1 == ub));

  ASSERT (  (-1 != ub));
  ASSERT (! ( 0 != ub));
  ASSERT (  ( 1 != ub));

  ASSERT (  (-1 <  INT_CAST ub));
  ASSERT (! ( 0 <  ub));

  ASSERT (  ( 0 <= ub));
  ASSERT (! ( 1 <= ub));

  ASSERT (! ( 0 >  ub));
  ASSERT (  ( 1 >  ub));

  ASSERT (! (-1 >= INT_CAST ub));
  ASSERT (  ( 0 >= ub));
#endif //__bool_true_false_are_defined
}

void
testFoo2(void)
{
  volatile struct {
      signed sb1:1;
      signed sb3:3;
    unsigned ub1:1;
    unsigned ub3:3;
  } str;

  str.sb1 = 0;
  str.ub1 = 0;
  str.sb3 = 0;
  str.ub3 = 0;

  ASSERT (! (-2 == str.sb1));
  ASSERT (! (-1 == str.sb1));
  ASSERT (  ( 0 == str.sb1));
  ASSERT (! ( 1 == str.sb1));

  ASSERT (! (-1 == str.ub1));
  ASSERT (  ( 0 == str.ub1));
  ASSERT (! ( 1 == str.ub1));
  ASSERT (! ( 2 == str.ub1));

  ASSERT (! (-5 == str.sb3));
  ASSERT (! (-4 == str.sb3));
  ASSERT (  ( 0 == str.sb3));
  ASSERT (! ( 3 == str.sb3));
  ASSERT (! ( 4 == str.sb3));

  ASSERT (! (-1 == str.ub3));
  ASSERT (  ( 0 == str.ub3));
  ASSERT (! ( 7 == str.ub3));
  ASSERT (! ( 8 == str.ub3));
}
