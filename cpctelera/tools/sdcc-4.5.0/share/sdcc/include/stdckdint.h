/*-------------------------------------------------------------------------
   stdckdint.h: Checked integer arithmetic

   Copyright (C) 2021, Philipp Klaus Krause, pkk@spth.de
   Copyright (C) 2023, Philipp Klaus Krause, philipp@colecovision.eu

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

#ifndef __SDCC_STDCKDINT_H
#define __SDCC_STDCKDINT_H 1

_Static_assert (sizeof(long long) >= 2 * sizeof(long));

// Default implementation: Suitable for additive operators for everything smaller than long long, and for multiplication for everything smaller than long long except for unsigned long times unsigned long. Not very efficient. Todo: Replace by more efficient implementation using _BitInt once all ports support _BitInt.
#define __CKD_DEFAULT_IMPL(T,O) \
(T *r, signed long long a, signed long long b) \
{ \
	signed long long result = a O b; \
	*r = result; \
	return (*r != result); \
}

#define __CKD_ULL_IMPL(T,O) \
(T *r, unsigned long long a, unsigned long long b) \
{ \
	unsigned long long result = a O b; \
	*r = result; \
	return (*r != result); \
}

inline _Bool __ckd_add_schar __CKD_DEFAULT_IMPL(signed char, +)
inline _Bool __ckd_add_uchar __CKD_DEFAULT_IMPL(unsigned char, +)
inline _Bool __ckd_add_short __CKD_DEFAULT_IMPL(short, +)
inline _Bool __ckd_add_ushort __CKD_DEFAULT_IMPL(unsigned short, +)
inline _Bool __ckd_add_int __CKD_DEFAULT_IMPL(int, +)
inline _Bool __ckd_add_uint __CKD_DEFAULT_IMPL(unsigned int, +)
inline _Bool __ckd_add_long __CKD_DEFAULT_IMPL(long, +)
inline _Bool __ckd_add_ulong __CKD_DEFAULT_IMPL(unsigned long, +)

inline _Bool __ckd_sub_schar __CKD_DEFAULT_IMPL(signed char, -)
inline _Bool __ckd_sub_uchar __CKD_DEFAULT_IMPL(unsigned char, -)
inline _Bool __ckd_sub_short __CKD_DEFAULT_IMPL(short, -)
inline _Bool __ckd_sub_ushort __CKD_DEFAULT_IMPL(unsigned short, -)
inline _Bool __ckd_sub_int __CKD_DEFAULT_IMPL(int, -)
inline _Bool __ckd_sub_uint __CKD_DEFAULT_IMPL(unsigned int, -)
inline _Bool __ckd_sub_long __CKD_DEFAULT_IMPL(long, -)
inline _Bool __ckd_sub_ulong __CKD_DEFAULT_IMPL(unsigned long, -)

inline _Bool __ckd_mul_schar __CKD_DEFAULT_IMPL(signed char, *)
inline _Bool __ckd_mul_uchar __CKD_DEFAULT_IMPL(unsigned char, *)
inline _Bool __ckd_mul_short __CKD_DEFAULT_IMPL(short, *)
inline _Bool __ckd_mul_ushort __CKD_DEFAULT_IMPL(unsigned short, *)
inline _Bool __ckd_mul_int __CKD_DEFAULT_IMPL(int, *)
inline _Bool __ckd_mul_uint __CKD_DEFAULT_IMPL(unsigned int, *)
inline _Bool __ckd_mul_long __CKD_DEFAULT_IMPL(long, *)
inline _Bool __ckd_mul_ulong __CKD_DEFAULT_IMPL(unsigned long, *)

inline _Bool __ckd_mul_ulongull __CKD_ULL_IMPL(unsigned long, *)

#if 0 // Elegant C, but inefficient asm - SDCC can't inline the calls!
#define __ckd_add_default(r, a, b) \
  _Generic ((r), \
    signed char * : __ckd_add_schar, \
    unsigned char * : __ckd_add_uchar, \
    short * : __ckd_add_short, \
    unsigned short * : __ckd_add_ushort, \
    int * : __ckd_add_int, \
    unsigned int * : __ckd_add_uint, \
    long * : __ckd_add_long, \
    unsigned long * : __ckd_add_ulong) \
    ((r), (a), (b))
#else
#define __ckd_add_default(r, a, b) \
  _Generic ((r), \
    signed char * : __ckd_add_schar((r), (a), (b)), \
    unsigned char * : __ckd_add_uchar((r), (a), (b)), \
    short * : __ckd_add_short((r), (a), (b)), \
    unsigned short * : __ckd_add_ushort((r), (a), (b)), \
    int * : __ckd_add_int((r), (a), (b)), \
    unsigned int * : __ckd_add_uint((r), (a), (b)), \
    long * : __ckd_add_long((r), (a), (b)), \
    unsigned long * : __ckd_add_ulong((r), (a), (b)))
#endif

// Elegant C, but inefficient asm - SDCC can't inline the calls! - but the alternative fails stm8 regression tests?
#define __ckd_sub_default(r, a, b) \
  _Generic ((r), \
    signed char * : __ckd_sub_schar, \
    unsigned char * : __ckd_sub_uchar, \
    short * : __ckd_sub_short, \
    unsigned short * : __ckd_sub_ushort, \
    int * : __ckd_sub_int, \
    unsigned int * : __ckd_sub_uint, \
    long * : __ckd_sub_long, \
    unsigned long * : __ckd_sub_ulong) \
    ((r), (a), (b))

#if 0 // Elegant C, but inefficient asm - SDCC can't inline the calls!
#define __ckd_mul_default(r, a, b) \
  _Generic ((r), \
    signed char * : __ckd_mul_schar, \
    unsigned char * : __ckd_mul_uchar, \
    short * : __ckd_mul_short, \
    unsigned short * : __ckd_mul_ushort, \
    int * : __ckd_mul_int, \
    unsigned int * : __ckd_mul_uint, \
    long * : __ckd_mul_long, \
    unsigned long * : __ckd_mul_ulong \
    ((r), (a), (b)))
#else
#define __ckd_mul_default(r, a, b) \
  _Generic ((r), \
    signed char * : __ckd_mul_schar((r), (a), (b)), \
    unsigned char * : __ckd_mul_uchar((r), (a), (b)), \
    short * : __ckd_mul_short((r), (a), (b)), \
    unsigned short * : __ckd_mul_ushort((r), (a), (b)), \
    int * : __ckd_mul_int((r), (a), (b)), \
    unsigned int * : __ckd_mul_uint((r), (a), (b)), \
    long * : __ckd_mul_long((r), (a), (b)), \
    unsigned long * : __ckd_mul_ulong((r), (a), (b)))
#endif


extern _Bool __ckd_add_unimplemented (void *, unsigned long long, unsigned long long);

#define ckd_add(r, a, b) \
  _Generic ((a), \
  signed long long: __ckd_add_unimplemented(r, a, b), \
  unsigned long long: __ckd_add_unimplemented(r, a, b), \
  default: \
    _Generic ((b), \
    signed long long: __ckd_add_unimplemented(r, a, b), \
    unsigned long long: __ckd_add_unimplemented(r, a, b), \
    default: __ckd_add_default(r, a, b))) \

extern _Bool __ckd_sub_unimplemented (void *, unsigned long long, unsigned long long);

#define ckd_sub(r, a, b) \
  _Generic ((a), \
  signed long long: __ckd_sub_unimplemented(r, a, b), \
  unsigned long long: __ckd_sub_unimplemented(r, a, b), \
  default: \
    _Generic ((b), \
    signed long long: __ckd_sub_unimplemented(r, a, b), \
    unsigned long long: __ckd_sub_unimplemented(r, a, b), \
    default: __ckd_sub_default(r, a, b))) \

extern _Bool __ckd_mul_unimplemented (void *, unsigned long long, unsigned long long);

#define ckd_mul(r, a, b) \
  _Generic ((a), \
  signed long long: __ckd_mul_unimplemented(r, a, b), \
  unsigned long long: __ckd_mul_unimplemented(r, a, b), \
  unsigned long: \
    _Generic ((b), \
    signed long long: __ckd_mul_unimplemented(r, a, b), \
    unsigned long long: __ckd_mul_unimplemented(r, a, b), \
    unsigned long: __ckd_mul_ulongull(r, a, b), \
    default: __ckd_mul_default(r, a, b)), \
  default: \
    _Generic ((b), \
    signed long long: __ckd_mul_unimplemented(r, a, b), \
    unsigned long long: __ckd_mul_unimplemented(r, a, b), \
    default: __ckd_mul_default(r, a, b))) \

#endif

