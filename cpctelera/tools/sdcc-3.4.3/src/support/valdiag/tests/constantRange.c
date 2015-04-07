#include <stdint.h>

volatile int8_t c;

/* sorry about the uggly source,
   but this way it can be the same as in the regression tests */
#define ASSERT(x) c = (x)

 int8_t s8;
uint8_t u8;

 int16_t s16;
uint16_t u16;

 int32_t s32;
uint32_t u32;

#ifdef TEST1
void foo(void)
{
  ASSERT (! (INT8_MIN - 1 == s8));		/* WARNING */
  ASSERT (! (INT8_MAX + 1 == s8));		/* WARNING */
  ASSERT (  (INT8_MIN - 1 != s8));		/* WARNING */
  ASSERT (  (INT8_MAX + 1 != s8));		/* WARNING */
  ASSERT (  (INT8_MIN - 1 <  s8));		/* WARNING */
  ASSERT (! (INT8_MAX     <  s8));		/* WARNING */
  ASSERT (  (INT8_MIN     <= s8));		/* WARNING */
  ASSERT (! (INT8_MAX + 1 <= s8));		/* WARNING */
  ASSERT (! (INT8_MIN     >  s8));		/* WARNING */
  ASSERT (  (INT8_MAX + 1 >  s8));		/* WARNING */
  ASSERT (! (INT8_MIN - 1 >= s8));		/* WARNING */
  ASSERT (  (INT8_MAX     >= s8));		/* WARNING */

  ASSERT (! (        0 - 1 == u8));		/* WARNING */
  ASSERT (! (UINT8_MAX + 1 == u8));		/* WARNING */
  ASSERT (  (        0 - 1 != u8));		/* WARNING */
  ASSERT (  (UINT8_MAX + 1 != u8));		/* WARNING */
  ASSERT (  (        0 - 1 <  u8));		/* WARNING */
  ASSERT (! (UINT8_MAX     <  u8));		/* WARNING */
  ASSERT (  (        0     <= u8));		/* WARNING */
  ASSERT (! (UINT8_MAX + 1 <= u8));		/* WARNING */
  ASSERT (! (        0     >  u8));		/* WARNING */
  ASSERT (  (UINT8_MAX + 1 >  u8));		/* WARNING */
  ASSERT (! (        0 - 1 >= u8));		/* WARNING */
  ASSERT (  (UINT8_MAX     >= u8));		/* WARNING */

  /* force extension to long to avoid int (16 bit) overflow */
  ASSERT (! (INT16_MIN - 1L == s16));		/* WARNING */
  ASSERT (! (INT16_MAX + 1L == s16));		/* WARNING */
  ASSERT (  (INT16_MIN - 1L != s16));		/* WARNING */
  ASSERT (  (INT16_MAX + 1L != s16));		/* WARNING */
  ASSERT (  (INT16_MIN - 1L <  s16));		/* WARNING */
  ASSERT (! (INT16_MAX      <  s16));		/* WARNING */
  ASSERT (  (INT16_MIN      <= s16));		/* WARNING */
  ASSERT (! (INT16_MAX + 1L <= s16));		/* WARNING */
  ASSERT (! (INT16_MIN      >  s16));		/* WARNING */
  ASSERT (  (INT16_MAX + 1L >  s16));		/* WARNING */
  ASSERT (! (INT16_MIN - 1L >= s16));		/* WARNING */
  ASSERT (  (INT16_MAX      >= s16));		/* WARNING */

  ASSERT (! (         0 - 1L == u16));		/* WARNING */
  ASSERT (! (UINT16_MAX + 1L == u16));		/* WARNING */
  ASSERT (  (         0 - 1L != u16));		/* WARNING */
  ASSERT (  (UINT16_MAX + 1L != u16));		/* WARNING */
  ASSERT (  (         0 - 1L <  u16));		/* WARNING */
  ASSERT (! (UINT16_MAX      <  u16));		/* WARNING */
  ASSERT (  (         0      <= u16));		/* WARNING */
  ASSERT (! (UINT16_MAX + 1L <= u16));		/* WARNING */
  ASSERT (! (         0      >  u16));		/* WARNING */
  ASSERT (  (UINT16_MAX + 1L >  u16));		/* WARNING */
  ASSERT (! (         0 - 1L >= u16));		/* WARNING */
  ASSERT (  (UINT16_MAX      >= u16));		/* WARNING */

   /* sdcc can't hold a number (INT32_MIN - 1) or (INT32_MAX + 1),
      there's no 'double' or 'long long' */
/* ASSERT (! (INT32_MIN - 1 == s32)); */
/* ASSERT (! (INT32_MAX + 1 == s32)); */
/* ASSERT (  (INT32_MIN - 1 != s32)); */
/* ASSERT (  (INT32_MAX + 1 != s32)); */
/* ASSERT (  (INT32_MIN - 1 <  s32)); */
   ASSERT (! (INT32_MAX     <  s32));		/* WARNING(SDCC) */
   ASSERT (  (INT32_MIN     <= s32));		/* WARNING(SDCC) */
/* ASSERT (! (INT32_MAX + 1 <= s32)); */
   ASSERT (! (INT32_MIN     >  s32));		/* WARNING(SDCC) */
/* ASSERT (  (INT32_MAX + 1 >  s32)); */
/* ASSERT (! (INT32_MIN - 1 >= s32)); */
   ASSERT (  (INT32_MAX     >= s32));		/* WARNING(SDCC) */

   /* (0 - 1) wraps around to UINT32_MAX -> untestable */
/* ASSERT (! (         0 - 1 == u32)); */
/* ASSERT (! (UINT32_MAX + 1 == u32)); */
/* ASSERT (  (         0 - 1 != u32)); */
/* ASSERT (  (UINT32_MAX + 1 != u32)); */
/* ASSERT (  (         0 - 1 <  u32)); */
   ASSERT (! (UINT32_MAX     <  u32));		/* WARNING(SDCC) */
   ASSERT (  (         0     <= u32));		/* WARNING(SDCC) */
/* ASSERT (! (UINT32_MAX + 1 <= u32)); */
   ASSERT (! (         0     >  u32));		/* WARNING(SDCC) */
/* ASSERT (  (UINT32_MAX + 1 >  u32)); */
/* ASSERT (! (         0 - 1 >= u32)); */
   ASSERT (  (UINT32_MAX     >= u32));		/* WARNING(SDCC) */
}
#endif

#ifdef TEST2
void foo(void)
{
   s8 = -129;		/* WARNING */
   s8 =  INT8_MIN;
   s8 = UINT8_MAX;
   s8 =  256;		/* WARNING */

   s8 = -129;		/* WARNING */
   u8 =  INT8_MIN;
   u8 = UINT8_MAX;
   u8 =  256;		/* WARNING */

   s16 = -32769L;	/* WARNING */
   s16 =  INT16_MIN;
   s16 = UINT16_MAX;
   s16 =  65536L;	/* WARNING */

   s16 = -32769L;	/* WARNING */
   u16 =  INT16_MIN;
   u16 = UINT16_MAX;
   u16 =  65536L;	/* WARNING */

   /* sdcc can't hold a number (INT32_MIN - 1) or (INT32_MAX + 1),
      there's no 'double' or 'long long' */
   s32 =  INT32_MIN;
   s32 = UINT32_MAX;

   u32 =  INT32_MIN;
   u32 = UINT32_MAX;
}
#endif

/* This test has been disabled. I don't think that signed/unsigned bool */
/* is a valid type. -- EEP */
#ifdef TEST3_DISABLED
#include <stdbool.h>

void foo(void)
{
#if defined(PORT_HOST)
   volatile bool sb, ub;
#else
   volatile   signed bool sb;
   volatile unsigned bool ub;
#endif

  sb = -2;
  sb = -1;
  sb =  0;
  sb =  1;

  ub = -1;
  ub =  0;
  ub =  1;
  ub =  2;

  ASSERT (! (-2 == sb));	/* WARNING(SDCC_mcs51|SDCC_ds390) */
  ASSERT (  (-1 == sb));
  ASSERT (  ( 0 == sb));
  ASSERT (! ( 1 == sb));	/* WARNING(SDCC_mcs51|SDCC_ds390) */

  ASSERT (  (-2 != sb));	/* WARNING(SDCC_mcs51|SDCC_ds390) */
  ASSERT (  (-1 != sb));
  ASSERT (  ( 0 != sb));
  ASSERT (  ( 1 != sb));	/* WARNING(SDCC_mcs51|SDCC_ds390) */

  ASSERT (  (-2 <  sb));	/* WARNING(SDCC_mcs51|SDCC_ds390) */
  ASSERT (  (-1 <  sb));
  ASSERT (! ( 0 <  sb));	/* WARNING(SDCC_mcs51|SDCC_ds390) */

  ASSERT (  (-1 <= sb));	/* WARNING(SDCC_mcs51|SDCC_ds390) */
  ASSERT (  ( 0 <= sb));
  ASSERT (! ( 1 <= sb));	/* WARNING(SDCC_mcs51|SDCC_ds390) */

  ASSERT (! (-1 >  sb));	/* WARNING(SDCC_mcs51|SDCC_ds390) */
  ASSERT (  ( 0 >  sb));
  ASSERT (  ( 1 >  sb));	/* WARNING(SDCC_mcs51|SDCC_ds390) */

  ASSERT (! (-2 >= sb));	/* WARNING(SDCC_mcs51|SDCC_ds390) */
  ASSERT (  (-1 >= sb));
  ASSERT (  ( 0 >= sb));	/* WARNING(SDCC_mcs51|SDCC_ds390) */


  ASSERT (! (-1 == ub));	/* WARNING(SDCC) */
  ASSERT (  ( 0 == ub));
  ASSERT (  ( 1 == ub));
  ASSERT (! ( 2 == ub));	/* WARNING(SDCC_mcs51|SDCC_ds390) */

  ASSERT (  (-1 != ub));	/* WARNING(SDCC) */
  ASSERT (  ( 0 != ub));
  ASSERT (  ( 1 != ub));
  ASSERT (  ( 2 != ub));	/* WARNING(SDCC_mcs51|SDCC_ds390) */

  ASSERT (  (-1 <  ub));	/* WARNING(SDCC) */
  ASSERT (  ( 0 <  ub));
  ASSERT (! ( 1 <  ub));	/* WARNING(SDCC_mcs51|SDCC_ds390) */

  ASSERT (  ( 0 <= ub));	/* WARNING(SDCC) */
  ASSERT (  ( 1 <= ub));
  ASSERT (! ( 2 <= ub));	/* WARNING(SDCC_mcs51|SDCC_ds390) */

  ASSERT (! ( 0 >  ub));	/* WARNING(SDCC) */
  ASSERT (  ( 1 >  ub));
  ASSERT (  ( 2 >  ub));	/* WARNING(SDCC_mcs51|SDCC_ds390) */

  ASSERT (! (-1 >= ub));	/* WARNING(SDCC) */
  ASSERT (  ( 0 >= ub));
  ASSERT (  ( 1 >= ub));	/* WARNING(SDCC_mcs51|SDCC_ds390) */
}
#endif

#ifdef TEST4
void foo(void)
{
  volatile struct {
      signed sb1:1;
      signed sb3:3;
    unsigned ub1:1;
    unsigned ub3:3;
  } str;

  str.sb1 = -2;			/* WARNING */
  str.sb1 = -1;
  str.sb1 =  1;
  str.sb1 =  2;			/* WARNING */

  str.ub1 = -2;			/* WARNING */
  str.ub1 = -1;
  str.ub1 =  1;
  str.ub1 =  2;			/* WARNING */

  str.sb3 = -5;			/* WARNING */
  str.sb3 = -4;
  str.sb3 =  7;
  str.sb3 =  8;			/* WARNING */

  str.ub3 = -5;			/* WARNING */
  str.ub3 = -4;
  str.ub3 =  7;
  str.ub3 =  8;			/* WARNING */

  ASSERT (! (-2 == str.sb1));	/* WARNING */
  ASSERT (  (-1 == str.sb1));
  ASSERT (  ( 0 == str.sb1));
  ASSERT (! ( 1 == str.sb1));	/* WARNING */

  ASSERT (! (-1 == str.ub1));	/* WARNING(SDCC) */
  ASSERT (  ( 0 == str.ub1));
  ASSERT (  ( 1 == str.ub1));
  ASSERT (! ( 2 == str.ub1));	/* WARNING(SDCC) */

  ASSERT (! (-5 == str.sb3));	/* WARNING */
  ASSERT (  (-4 == str.sb3));
  ASSERT (  ( 3 == str.sb3));
  ASSERT (! ( 4 == str.sb3));	/* WARNING */

  ASSERT (! (-1 == str.ub3));	/* WARNING(SDCC) */
  ASSERT (  ( 0 == str.ub3));
  ASSERT (  ( 7 == str.ub3));
  ASSERT (! ( 8 == str.ub3));	/* WARNING(SDCC) */
}
#endif
