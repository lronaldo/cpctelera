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

#ifndef __SDCC_STDLIB_H
#define __SDCC_STDLIB_H 1

#ifndef __SIZE_T_DEFINED
#define __SIZE_T_DEFINED
  typedef unsigned int size_t;
#endif

#ifndef __WCHAR_T_DEFINED
#define __WCHAR_T_DEFINED
  typedef unsigned long int wchar_t;
#endif

#ifndef NULL
#define NULL (void *)0
#endif

#if defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_r2k) || defined(__SDCC_r3ka) || defined(__SDCC_tlcs90)
int abs(int j) __preserves_regs(b, c, iyl, iyh);
#else
int abs(int j);
#endif
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

#if defined(__SDCC_mcs51) || defined(__SDCC_ds390) || defined(__SDCC_ds400)
void __xdata *calloc (size_t nmemb, size_t size);
void __xdata *malloc (size_t size);
void __xdata *realloc (void *ptr, size_t size);
#else
void *calloc (size_t nmemb, size_t size);
void *malloc (size_t size);
void *realloc (void *ptr, size_t size);
#endif

#if __STDC_VERSION__ >= 201112L
inline void *aligned_alloc(size_t alignment, size_t size)
{
  return malloc(size);
}
#endif

extern void free (void * ptr);

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

