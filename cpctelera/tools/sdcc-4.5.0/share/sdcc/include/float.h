/*-------------------------------------------------------------------------
   float.h - ANSI functions forward declarations

   Copyright (C) 1998, Sandeep Dutta . sandeep.dutta@usa.net

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this library; see the file COPYING. If not, write to the
   Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
   MA 02110-1301, USA.

   As a special exception, if you link this library with other files,
   some of which are compiled with SDCC, to produce an executable,
   this library does not by itself cause the resulting executable to
   be covered by the GNU General Public License. This exception does
   not however invalidate any other reasons why the executable file
   might be covered by the GNU General Public License.
-------------------------------------------------------------------------*/

#ifndef __SDC51_FLOAT_H
#define __SDC51_FLOAT_H 1

#include <limits.h>

#include <sdcc-lib.h>

#define FLT_RADIX       2
#define FLT_MANT_DIG    24
#define FLT_EPSILON     1.192092896E-07F
#define FLT_DIG         6
#define FLT_MIN_EXP     (-125)
#define FLT_MIN         1.175494351E-38F
#define FLT_MIN_10_EXP  (-37)
#define FLT_MAX_EXP     (+128)
#define FLT_MAX         3.402823466E+38F
#define FLT_MAX_10_EXP  (+38)
#if __STDC_VERSION__ >= 201112L
#define FLT_DECIMAL_DIG 9
#define FLT_TRUE_MIN    1.175494351E-38F
#define FLT_HAS_SUBNORM 0
#endif

#if __SIZEOF_DOUBLE__ == 4

#define DBL_MANT_DIG    FLT_MANT_DIG
#define DBL_EPSILON     FLT_EPSILON
#define DBL_DIG         FLT_DIG
#define DBL_MIN_EXP     FLT_MIN_EXP
#define DBL_MIN         FLT_MIN
#define DBL_MIN_10_EXP  FLT_MIN_10_EXP
#define DBL_MAX_EXP     FLT_MAX_EXP
#define DBL_MAX         FLT_MAX
#define DBL_MAX_10_EXP  FLT_MAX_10_EXP
#if __STDC_VERSION__ >= 201112L
#define DBL_DECIMAL_DIG FLT_DECIMAL_DIG
#define DBL_TRUE_MIN    FLT_TRUE_MIN
#define DBL_HAS_SUBNORM FLT_HAS_SUBNORM
#endif

#endif

/* the following deal with IEEE single-precision numbers */
#if defined(__SDCC_FLOAT_LIB)
#define EXCESS		126
#define SIGNBIT		((unsigned long)0x80000000)
#define __INFINITY	((unsigned long)0x7F800000)
#define __NAN       ((unsigned long)0xFFC00000)
#define HIDDEN		(unsigned long)(1ul << 23)
#define SIGN(fp)	(((unsigned long)(fp) >> (8*sizeof(fp)-1)) & 1)
#define EXP(fp)		(((unsigned long)(fp) >> 23) & (unsigned int) 0x00FF)
#define MANT(fp)	(((fp) & (unsigned long)0x00FFFFFF) | HIDDEN)
#define NORM            0xff000000
#define PACK(s,e,m)	((s) | ((unsigned long)(e) << 23) | (m))
#endif

#define __SDCC_FLOAT_NONBANKED __SDCC_NONBANKED

float __uchar2fs (unsigned char) __SDCC_FLOAT_NONBANKED;
float __schar2fs (signed char) __SDCC_FLOAT_NONBANKED;
float __uint2fs (unsigned int) __SDCC_FLOAT_NONBANKED;
float __sint2fs (signed int) __SDCC_FLOAT_NONBANKED;
float __ulong2fs (unsigned long) __SDCC_FLOAT_NONBANKED;
float __slong2fs (signed long) __SDCC_FLOAT_NONBANKED;
#ifdef __SDCC_LONGLONG
float __ulonglong2fs (unsigned long long) __SDCC_FLOAT_NONBANKED;
float __slonglong2fs (signed long long) __SDCC_FLOAT_NONBANKED;
#endif
unsigned char __fs2uchar (float) __SDCC_FLOAT_NONBANKED;
signed char __fs2schar (float) __SDCC_FLOAT_NONBANKED;
unsigned int __fs2uint (float) __SDCC_FLOAT_NONBANKED;
signed int __fs2sint (float) __SDCC_FLOAT_NONBANKED;
unsigned long __fs2ulong (float) __SDCC_FLOAT_NONBANKED;
signed long __fs2slong (float) __SDCC_FLOAT_NONBANKED;

float __fsadd (float, float) __SDCC_FLOAT_NONBANKED;
float __fssub (float, float) __SDCC_FLOAT_NONBANKED;
float __fsmul (float, float) __SDCC_FLOAT_NONBANKED;
float __fsdiv (float, float) __SDCC_FLOAT_NONBANKED;

_Bool __fslt (float, float) __SDCC_FLOAT_NONBANKED;
_Bool __fseq (float, float) __SDCC_FLOAT_NONBANKED;
_Bool __fsgt (float, float) __SDCC_FLOAT_NONBANKED;


#if defined(__SDCC_FLOAT_LIB) && defined(__SDCC_mcs51) && !defined(__SDCC_USE_XSTACK) && !defined(_SDCC_NO_ASM_LIB_FUNCS)

#define FLOAT_ASM_MCS51

/* This adds extra code for proper round-off, in
   an attempt to match the results from gcc. */
#define FLOAT_FULL_ACCURACY

/* This adds about 66 bytes to the code size and
   significantly speeds up shift operations more
   than 8 bits (common when subtracting numbers
   of significantly different magnitude and scaling
   to fixed point) */
#define FLOAT_SHIFT_SPEEDUP

#define sign_a  psw.1
#define sign_b  psw.5
#define exp_a dpl
#define exp_b dph
#endif	/* using mcs51 assembly */


#endif	/* __SDC51_FLOAT_H */

