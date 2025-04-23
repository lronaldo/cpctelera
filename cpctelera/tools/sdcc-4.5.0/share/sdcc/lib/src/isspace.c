/*-------------------------------------------------------------------------
   isspace.c

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

#ifdef isspace
#undef isspace
#endif

// ' ' and '\n' are most common; '\t' to '\r' ordered by value
int isspace (int c)
{
#if defined ( __SDCC_sm83 ) || defined ( __SDCC_z80 ) ||  defined ( __SDCC_z80n ) ||  defined ( __SDCC_z180 ) ||  defined ( __SDCC_ez80_z80 ) ||  defined ( __SDCC_mos6502 ) 
  if((c & 0xff00) != 0)
    return 0;
  return ((unsigned char)c == ' ' || (unsigned char)c == '\t' || (unsigned char)c == '\n' || (unsigned char)c == '\v' || (unsigned char)c == '\f' || (unsigned char)c == '\r');
#else
  return (c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r');
#endif
}

