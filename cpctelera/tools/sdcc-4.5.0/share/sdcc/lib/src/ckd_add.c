/*-------------------------------------------------------------------------
   ckd_add.c - checked integer addition.

   Copyright (C) 2022, Philipp Klaus Krause, krauseph@informatik.uni-freiburg.de

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

#pragma std_c23

/* it is important to declare these functions extern before including
   the inline definition to give it external linkage */

#define __CKD_DEFAULT_DECL(T,O) \
(T *r, signed long long a, signed long long b)

extern _Bool __ckd_add_schar __CKD_DEFAULT_DECL(signed char, +);
extern _Bool __ckd_add_uchar __CKD_DEFAULT_DECL(unsigned char, +);
extern _Bool __ckd_add_short __CKD_DEFAULT_DECL(short, +);
extern _Bool __ckd_add_ushort __CKD_DEFAULT_DECL(unsigned short, +);
extern _Bool __ckd_add_int __CKD_DEFAULT_DECL(int, +);
extern _Bool __ckd_add_uint __CKD_DEFAULT_DECL(unsigned int, +);
extern _Bool __ckd_add_long __CKD_DEFAULT_DECL(long, +);
extern _Bool __ckd_add_ulong __CKD_DEFAULT_DECL(unsigned long, +);

#include <stdckdint.h>

