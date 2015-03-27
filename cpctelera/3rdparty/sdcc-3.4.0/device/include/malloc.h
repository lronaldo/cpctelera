/*-------------------------------------------------------------------------
   malloc.h - malloc header file

   Copyright (C) 1997, Dmitry S. Obukhov <dmitry.obukhov AT gmail.com>

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

#ifndef __SDCC51_MALLOC_H
#define __SDCC51_MALLOC_H
#include <sdcc-lib.h>
#include <stddef.h>

#if _SDCC_MALLOC_TYPE_MLH

void * calloc (size_t nmemb, size_t size);
void * malloc (size_t size);
void * realloc (void * ptr, size_t size);
void free (void * ptr);

#else

extern void __xdata * calloc (size_t nmemb, size_t size);
extern void __xdata * malloc (size_t size);
extern void __xdata * realloc (void * ptr, size_t size);
extern void free (void * ptr);

#endif

#endif
