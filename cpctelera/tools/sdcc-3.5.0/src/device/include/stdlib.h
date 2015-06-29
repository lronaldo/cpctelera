/*-------------------------------------------------------------------------
   stdlib.h - ANSI functions forward declarations

   Copyright (C)1998, Sandeep Dutta . sandeep.dutta@usa.net

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

#ifndef __SDC51_STDLIB_H
#define __SDC51_STDLIB_H 1

#ifndef NULL
# define NULL (void *)0
#endif

#include <malloc.h>

int abs(int j);
long int labs(long int j);

extern float atof (const char *nptr);
extern int atoi (const char *nptr);
extern long int atol (const char *nptr);
#ifdef __SDCC_LONGLONG
extern long long int atoll (const char *nptr);
#endif

extern void _uitoa(unsigned int, char*, unsigned char);
extern void _itoa(int, char*, unsigned char);

extern void _ultoa(unsigned long, char*, unsigned char);
extern void _ltoa(long, char*, unsigned char);

#define RAND_MAX 32767

int rand(void);
void srand(unsigned int seed);

/* Bounds-checking interfaces from annex K of the C11 standard. */
#if defined (__STDC_WANT_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__

#ifndef __RSIZE_T_DEFINED
#define __RSIZE_T_DEFINED
typedef size_t rsize_t;
#endif

#ifndef __ERRNO_T_DEFINED
#define __ERRNO_T_DEFINED
typedef int errno_t;
#endif

typedef void (*constraint_handler_t)(const char *restrict msg, void *restrict ptr, errno_t error);

#endif

#endif
