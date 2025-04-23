/*-------------------------------------------------------------------------
   stdarg.h - ANSI macros for variable parameter list

   Copyright (C) 2000, Michael Hope
   Copyright (C) 2011, Philipp Klaus Krause pkk@spth.de

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

#ifndef __STDC_VERSION_STDARG_H__
#define __STDC_VERSION_STDARG_H__ 201710L /* TODO: replace by __STDC_VERSION__ when this header becomes C23-compliant! */

#if defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_r2k) || defined(__SDCC_r2ka) || defined(__SDCC_r3ka) || defined(__SDCC_tlcs90) || defined (__SDCC_ez80_z80) || defined (__SDCC_z80n) || defined(__SDCC_sm83) || defined(__SDCC_r800) || defined(__SDCC_hc08) || defined(__SDCC_s08) || defined(__SDCC_mos6502) || defined(__SDCC_mos65c02) || defined(__SDCC_stm8) || defined(__SDCC_pic16) || defined(__SDCC_f8)

typedef unsigned char * va_list;
#define va_start(marker, last)  { marker = (va_list)&last + sizeof(last); }
#define va_arg(marker, type)    *((type *)((marker += sizeof(type)) - sizeof(type)))

#elif defined(__SDCC_ds390) || defined(__SDCC_ds400)

typedef unsigned char * va_list;
#define va_start(marker, first) { marker = (va_list)&first; }
#define va_arg(marker, type)    *((type *)(marker -= sizeof(type)))

#elif defined(__SDCC_pdk13) || defined(__SDCC_pdk14) || defined(__SDCC_pdk15)

typedef unsigned char * va_list;
#define va_start(marker, first) { marker = (va_list)&first; }
#define va_arg(marker, type)    *((type *)(marker -= (sizeof(type) + sizeof(type) % 2)))

#elif defined(__SDCC_USE_XSTACK)

typedef unsigned char __pdata * va_list;
#define va_start(marker, first) { marker = (va_list)&first; }
#define va_arg(marker, type)    *((type __pdata *)(marker -= sizeof(type)))

#else

typedef unsigned char __data * va_list;
#define va_start(marker, first) { marker = (va_list)&first; }
#define va_arg(marker, type)    *((type __data * )(marker -= sizeof(type)))

#endif

#define va_copy(dest, src)      { dest = src; }
#define va_end(marker)          { marker = (va_list) 0; };

#endif

