/*-------------------------------------------------------------------------
   calloc.c - allocate cleared memory.

   Copyright (C) 2004, Maarten Brock, sourceforge.brock@dse.nl

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

#include <sdcc-lib.h>
#include <malloc.h>
#include <string.h>

//--------------------------------------------------------------------
//calloc function implementation for embedded system
//Non-ANSI keywords are C51 specific.
// __xdata - variable in external memory (just RAM)
//--------------------------------------------------------------------

#if _SDCC_MALLOC_TYPE_MLH

#define __xdata

#endif

void __xdata * calloc (size_t nmemb, size_t size)
{
  register void __xdata * ptr;

  ptr = malloc(nmemb * size);
  if (ptr)
  {
    memset(ptr, 0, nmemb * size);
  }
  return ptr;
}
//END OF MODULE
