/*-------------------------------------------------------------------------
   isxdigit.c

   Philipp Klaus Krause, philipp@informatik.uni-frankfurt.de 2013

   (c) 2013 Goethe-Universität Frankfurt
   (c) 2022 Sebastian 'basxto' Riedel

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

#include <ctype.h>

int isxdigit (int c)
{
#if defined ( __SDCC_sm83 ) || defined ( __SDCC_z80 ) ||  defined ( __SDCC_z80n ) ||  defined ( __SDCC_z180 ) ||  defined ( __SDCC_ez80_z80 ) ||  defined ( __SDCC_mos6502 ) 
  if((c & 0xff00) != 0)
    return 0;
  return ((unsigned char)c >= '0' && (unsigned char)c <= '9' || (unsigned char)c >= 'a' && (unsigned char)c <= 'f' || (unsigned char)c >= 'A' && (unsigned char)c <= 'F');
#else
  return (c >= '0' && c <= '9' || c >= 'a' && c <= 'f' || c >= 'A' && c <= 'F');
#endif
}

