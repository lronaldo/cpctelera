/* ------------------------------------------------------------------------
   _divuchar.c - 8 bit division routines for pic14 devices

   Copyright (C) 2005, Raphael Neider <rneider AT web.de>

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

unsigned char
_divuchar (unsigned char a, unsigned char b)
{
  unsigned char result = 0;
  unsigned char mask = 0x01;

  /* prevent endless loop (division by zero exception?!?) */
  if (!b) return (unsigned char)-1;

  /* it would suffice to make b >= a, but that test is
   * more complex and will fail if a has its MSB set */
  while (!(b & (1UL << (8*sizeof(unsigned char)-1)))) {
    b <<= 1;
    mask <<= 1;
  } // while

  /* now add up the powers of two (of b) that "fit" into a */
  /* we might stop if (a == 0), but that's an additional test in every iteration... */
  while (mask) {
    if (a >= b) {
      result += mask;
      a -= b;
    } // if
    b >>= 1;
    mask >>= 1;
  } // while

  return result;
}

