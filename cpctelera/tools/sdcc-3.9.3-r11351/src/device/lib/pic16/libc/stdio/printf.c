/*-------------------------------------------------------------------------
   printf.c

   Copyright (C) 2005, Vangelis Rokas <vrokas AT otenet.gr>

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

#include <stdarg.h>
#include <stdio.h>

#if _DEBUG
extern void io_long (unsigned long);
extern void io_str (char *);
#endif

int
printf (const char *fmt, ...)
{
  va_list ap;

#if _DEBUG
  io_str ("printf: ");
  io_long ((unsigned long) stdout);
#endif

  va_start (ap, fmt);
  return vfprintf (stdout, fmt, ap);
}
