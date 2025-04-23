/*---------------------------------------------------------------------
   wcstol() - convert a wide string to a long int and return it

   Copyright (C) 2018-2023, Philipp Klaus Krause . krauseph@informatik.uni-freiburg.de
                 2023, Benedikt Freisen . b.freisen@gmx.net

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

#include <stdlib.h>

#include <stdbool.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <wchar.h>

#pragma disable_warning 196

long int wcstol(const wchar_t *restrict nptr, wchar_t **restrict endptr, int base)
{
  const wchar_t *ptr = nptr;
  const wchar_t *rptr;
  unsigned long int u;
  bool neg;

  while (iswblank (*ptr))
    ptr++;

  neg = (*ptr == L'-');

  if (*ptr == L'-')
    {
      neg = true;
      ptr++;
    }
  else
    neg = false;

  // wcstoul() would accept leading blanks or signs (that might come after L'-' handled above)
  if (neg && (iswblank (*ptr) || *ptr == L'-' || *ptr == L'+'))
    {
      if (endptr)
        *endptr = nptr;
      return (0);
    }

  u = wcstoul(ptr, &rptr, base);

  // Check for conversion error
  if (rptr == ptr)
    {
      if (endptr)
        *endptr = nptr;
      return (0);
    }

  if (endptr)
    *endptr = rptr;

  // Check for range error
  if (!neg && u > LONG_MAX)
    {
      errno = ERANGE;
      return (LONG_MAX);
    }
  else if (neg && u > -LONG_MIN)
    {
      errno = ERANGE;
      return (LONG_MIN);
    }

  return (neg ? -u : u);
}

