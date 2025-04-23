/*-------------------------------------------------------------------------
   inttypes.h - ISO C99 7.8 Format conversion of integer types <inttypes.h>

   Copyright (C) 2019-2023, Benedikt Freisen, b.freisen@gmx.net

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

#ifndef _INTTYPES_H
#define _INTTYPES_H 1

/* the relevant integer types are defined in stdint.h */

#include <stdint.h>

/* conversion specifiers suitable for int8_t and uint8_t */

#define PRId8       "hhd"
#define PRIi8       "hhi"
#define PRIo8       "hho"
#define PRIu8       "hhu"
#define PRIx8       "hhx"
#define PRIX8       "hhX"
#define SCNd8       "hhd"
#define SCNi8       "hhi"
#define SCNo8       "hho"
#define SCNu8       "hhu"
#define SCNx8       "hhx"

/* conversion specifiers suitable for int_least8_t and uint_least8_t */

#define PRIdLEAST8  "hhd"
#define PRIiLEAST8  "hhi"
#define PRIoLEAST8  "hho"
#define PRIuLEAST8  "hhu"
#define PRIxLEAST8  "hhx"
#define PRIXLEAST8  "hhX"
#define SCNdLEAST8  "hhd"
#define SCNiLEAST8  "hhi"
#define SCNoLEAST8  "hho"
#define SCNuLEAST8  "hhu"
#define SCNxLEAST8  "hhx"

/* conversion specifiers suitable for int_fast8_t and uint_fast8_t */

#define PRIdFAST8   "hhd"
#define PRIiFAST8   "hhi"
#define PRIoFAST8   "hho"
#define PRIuFAST8   "hhu"
#define PRIxFAST8   "hhx"
#define PRIXFAST8   "hhX"
#define SCNdFAST8   "hhd"
#define SCNiFAST8   "hhi"
#define SCNoFAST8   "hho"
#define SCNuFAST8   "hhu"
#define SCNxFAST8   "hhx"

/* conversion specifiers suitable for int16_t and uint16_t */

#define PRId16      "hd"
#define PRIi16      "hi"
#define PRIo16      "ho"
#define PRIu16      "hu"
#define PRIx16      "hx"
#define PRIX16      "hX"
#define SCNd16      "hd"
#define SCNi16      "hi"
#define SCNo16      "ho"
#define SCNu16      "hu"
#define SCNx16      "hx"

/* conversion specifiers suitable for int_least_16_t and uint_least16_t */

#define PRIdLEAST16 "hd"
#define PRIiLEAST16 "hi"
#define PRIoLEAST16 "ho"
#define PRIuLEAST16 "hu"
#define PRIxLEAST16 "hx"
#define PRIXLEAST16 "hX"
#define SCNdLEAST16 "hd"
#define SCNiLEAST16 "hi"
#define SCNoLEAST16 "ho"
#define SCNuLEAST16 "hu"
#define SCNxLEAST16 "hx"

/* conversion specifiers suitable for int_fast16_t and uint_fast16_t */

#define PRIdFAST16  "hd"
#define PRIiFAST16  "hi"
#define PRIoFAST16  "ho"
#define PRIuFAST16  "hu"
#define PRIxFAST16  "hx"
#define PRIXFAST16  "hX"
#define SCNdFAST16  "hd"
#define SCNiFAST16  "hi"
#define SCNoFAST16  "ho"
#define SCNuFAST16  "hu"
#define SCNxFAST16  "hx"

/* conversion specifiers suitable for int32_t and uint32_t */

#define PRId32      "ld"
#define PRIi32      "li"
#define PRIo32      "lo"
#define PRIu32      "lu"
#define PRIx32      "lx"
#define PRIX32      "lX"
#define SCNd32      "ld"
#define SCNi32      "li"
#define SCNo32      "lo"
#define SCNu32      "lu"
#define SCNx32      "lx"

/* conversion specifiers suitable for int_least32_t and uint_least32_t */

#define PRIdLEAST32 "ld"
#define PRIiLEAST32 "li"
#define PRIoLEAST32 "lo"
#define PRIuLEAST32 "lu"
#define PRIxLEAST32 "lx"
#define PRIXLEAST32 "lX"
#define SCNdLEAST32 "ld"
#define SCNiLEAST32 "li"
#define SCNoLEAST32 "lo"
#define SCNuLEAST32 "lu"
#define SCNxLEAST32 "lx"

/* conversion specifiers suitable for int_fast32_t and uint_fast32_t */

#define PRIdFAST32  "ld"
#define PRIiFAST32  "li"
#define PRIoFAST32  "lo"
#define PRIuFAST32  "lu"
#define PRIxFAST32  "lx"
#define PRIXFAST32  "lX"
#define SCNdFAST32  "ld"
#define SCNiFAST32  "li"
#define SCNoFAST32  "lo"
#define SCNuFAST32  "lu"
#define SCNxFAST32  "lx"

/* conversion specifiers suitable for int64_t and uint64_t */

#define PRId64      "lld"
#define PRIi64      "lli"
#define PRIo64      "llo"
#define PRIu64      "llu"
#define PRIx64      "llx"
#define PRIX64      "llX"
#define SCNd64      "lld"
#define SCNi64      "lli"
#define SCNo64      "llo"
#define SCNu64      "llu"
#define SCNx64      "llx"

/* conversion specifiers suitable for int_least64_t and uint_least64_t */

#define PRIdLEAST64 "lld"
#define PRIiLEAST64 "lli"
#define PRIoLEAST64 "llo"
#define PRIuLEAST64 "llu"
#define PRIxLEAST64 "llx"
#define PRIXLEAST64 "llX"
#define SCNdLEAST64 "lld"
#define SCNiLEAST64 "lli"
#define SCNoLEAST64 "llo"
#define SCNuLEAST64 "llu"
#define SCNxLEAST64 "llx"

/* conversion specifiers suitable for int_fast64_t and uint_fast64_t */

#define PRIdFAST64  "lld"
#define PRIiFAST64  "lli"
#define PRIoFAST64  "llo"
#define PRIuFAST64  "llu"
#define PRIxFAST64  "llx"
#define PRIXFAST64  "llX"
#define SCNdFAST64  "lld"
#define SCNiFAST64  "lli"
#define SCNoFAST64  "llo"
#define SCNuFAST64  "llu"
#define SCNxFAST64  "llx"

/* conversion specifiers suitable for intmax_t and uintmax_t */

#define PRIdMAX     "jd"
#define PRIiMAX     "ji"
#define PRIoMAX     "jo"
#define PRIuMAX     "ju"
#define PRIxMAX     "jx"
#define PRIXMAX     "jX"
#define SCNdMAX     "jd"
#define SCNiMAX     "ji"
#define SCNoMAX     "jo"
#define SCNuMAX     "ju"
#define SCNxMAX     "jx"

/* conversion specifiers suitable for intptr_t and uintptr_t */

#define PRIdPTR     "td"
#define PRIiPTR     "ti"
#define PRIoPTR     "to"
#define PRIuPTR     "tu"
#define PRIxPTR     "tx"
#define PRIXPTR     "tX"
#define SCNdPTR     "td"
#define SCNiPTR     "ti"
#define SCNoPTR     "to"
#define SCNuPTR     "tu"
#define SCNxPTR     "tx"

/* result type of the imaxdiv function */

typedef struct {
  intmax_t quot;
  intmax_t rem;
} imaxdiv_t;

/* declarations of functions related to intmax_t and uintmax_t */

intmax_t imaxabs(intmax_t j);

/* NOTE: SDCC does not currently support struct return types on these architectures */
#if !defined(__SDCC_ds390) && !defined(__SDCC_hc08) && !defined(__SDCC_s08) && !defined(__SDCC_mos6502) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
imaxdiv_t imaxdiv(intmax_t numer, intmax_t denom);
#endif

intmax_t strtoimax(const char * restrict nptr, char ** restrict endptr, int base);
uintmax_t strtoumax(const char * restrict nptr, char ** restrict endptr, int base);

#ifdef __WCHAR_T_DEFINED
intmax_t wcstoimax(const wchar_t * restrict nptr, wchar_t ** restrict endptr, int base);
uintmax_t wcstoumax(const wchar_t * restrict nptr, wchar_t ** restrict endptr, int base);
#endif

#endif /* inttypes.h */
