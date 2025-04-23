/*-------------------------------------------------------------------------
   _srulonglong.c - routine for shift right of 64 bit unsigned long long

   Copyright (C) 2012, Philipp Klaus Krause . philipp@informatik.uni-frankfurt.de

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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

#include <stdint.h>

#include <stdbit.h>

#include <sdcc-lib.h>

#ifdef __SDCC_LONGLONG

#if __STDC_ENDIAN_NATIVE__ == __STDC_ENDIAN_BIG__
unsigned long long _srulonglong(unsigned long long l, char s) __SDCC_NONBANKED
{
    uint32_t *const top =    (uint32_t *)((char *)(&l) + 0);
    uint32_t *const middle = (uint16_t *)((char *)(&l) + 2);
    uint32_t *const bottom = (uint32_t *)((char *)(&l) + 4);
    uint16_t *const w =      (uint16_t *)(&l);

    for (; s >= 16; s -= 16)
    {
        w[3] = w[2];
        w[2] = w[1];
        w[1] = w[0];
        w[0] = 0x000000;
    }

    (*bottom) >>= s;
    (*middle) |= (((*middle & 0xffff0000ul) >> s) & 0x0000fffful);
    (*top) >>= s;

    return(l);
}
#elif __STDC_ENDIAN_NATIVE__ == __STDC_ENDIAN_LITTLE__
unsigned long long _srulonglong(unsigned long long l, char s) __SDCC_NONBANKED
{
    _AUTOMEM uint32_t *const top =    (_AUTOMEM uint32_t *)((_AUTOMEM char *)(&l) + 4);
    _AUTOMEM uint16_t *const middle = (_AUTOMEM uint16_t *)((_AUTOMEM char *)(&l) + 4);
    _AUTOMEM uint32_t *const bottom = (_AUTOMEM uint32_t *)((_AUTOMEM char *)(&l) + 0);
    _AUTOMEM uint16_t *const w =      (_AUTOMEM uint16_t *)(&l);

    for (; s >= 16; s -= 16)
    {
        w[0] = w[1];
        w[1] = w[2];
        w[2] = w[3];
        w[3] = 0x000000;
    }

    (*bottom) >>= s;
    (*middle) |= (uint16_t)(((uint32_t)(*middle) << 16) >> s);
    (*top) |= (((*middle) & 0xffff0000ul) >> s);

    return(l);
}
#else
#error Support for mixed endiannness not implemented!
#endif

#endif

