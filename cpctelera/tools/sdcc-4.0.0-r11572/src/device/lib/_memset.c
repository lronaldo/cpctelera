/*-------------------------------------------------------------------------
   _memset.c - part of string library functions

   Copyright (C) 1999, Sandeep Dutta . sandeep.dutta@usa.net
   mcs51 assembler by Frieder Ferlemann (2007)

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

#include <string.h>

#undef memset /* Avoid conflict with builtin memset() in Z80 and some related ports */

#if defined (_SDCC_NO_ASM_LIB_FUNCS) || !defined (__SDCC_mcs51) || \
    (!defined (__SDCC_MODEL_SMALL) && !defined (__SDCC_MODEL_LARGE)) || \
     (defined (__SDCC_STACK_AUTO) || defined (__SDCC_PARMS_IN_BANK1) )

#ifdef __SDCC_BROKEN_STRING_FUNCTIONS
void *memset (void *s, unsigned char c, size_t n)
#else
void *memset (void *s, int c, size_t n)
#endif
{
 register unsigned char *ret = s;

 while (n--)
   {
      *(unsigned char *) ret = c;
      ret = ((unsigned char *) ret) + 1;
   }

   return s;
}

#else

  /* assembler implementation for mcs51 */
  static void dummy(void) __naked
  {
    __asm

  /* assigning function parameters to registers.
     __SDCC_PARMS_IN_BANK1 or __SDCC_STACK_AUTO not yet implemented. */
  #if defined (__SDCC_MODEL_SMALL)

    #if defined(__SDCC_NOOVERLAY)
        .area DSEG    (DATA)
    #else
        .area OSEG    (OVR,DATA)
    #endif
        _memset_PARM_2::
              .ds 1
        _memset_PARM_3::
              .ds 2

        .area CSEG    (CODE)

        _memset::

        ;   Assign buf (b holds memspace, no need to touch)
                mov     r4,dpl
                mov     r5,dph
                ;
        ;   Assign count
                mov     r6,_memset_PARM_3
                mov     r7,(_memset_PARM_3 + 1)
                ;
        ;   if (!count) return buf;
        ;   check for count != 0 intermangled with gymnastic
        ;   preparing djnz instructions
                cjne    r6,#0x00,COUNT_LSB_NOT_ZERO
                mov     a,r7
                jz      MEMSET_END
                dec     r7
        COUNT_LSB_NOT_ZERO:
                inc     r7
                ;
                ; This was 8 byte overhead for preparing
                ; the count argument for an integer loop with two
                ; djnz instructions - it might make sense to
                ; let SDCC automatically generate this when
                ; it encounters a loop like:
                ; for(i=0;i<j;i++){...}
                ; (at least for option --opt-code-speed)
                ;

        ;   Assign ch
                mov     a,_memset_PARM_2

  #else

        .area XSEG    (XDATA)

        _memset_PARM_2::
                .ds 1
        _memset_PARM_3::
                .ds 2

        .area CSEG    (CODE)

        _memset::

        ;   Assign buf (b holds memspace, no need to touch)
                mov     r4,dpl
                mov     r5,dph
                ;
        ;   Assign count
                mov     dptr,#_memset_PARM_3
                movx    a,@dptr
                mov     r6,a
                inc     dptr
                movx    a,@dptr
                mov     r7,a
                ;
        ;   if (!count) return buf;
        ;   check for count != 0 intermangled with gymnastic
        ;   preparing djnz instructions
                cjne    r6,#0x00,COUNT_LSB_NOT_ZERO
        ;   acc holds r7
                jz      MEMSET_END
                dec     r7
        COUNT_LSB_NOT_ZERO:
                inc     r7
                ;
        ;   Assign ch
                mov     dptr,#_memset_PARM_2
                movx    a,@dptr
        ;   acc is precious now
                ;
        ;   Restore dptr
                mov     dpl,r4
                mov     dph,r5

  #endif

        /* now independent of the parameter passing everything
           should be in registers by now and the loop may start */
        ;   _memset.c do {

        MEMSET_LOOP:
        ;   _memset.c *p = ch;
                lcall   __gptrput

        ;   _memset.c p++;
                inc     dptr

        ;   _memset.c } while(--count) ;
                djnz    r6,MEMSET_LOOP
                djnz    r7,MEMSET_LOOP
                ;

        MEMSET_END:
        ;   _memset.c return buf ;
                ; b was unchanged
                mov     dpl,r4
                mov     dph,r5
                ;
                ret

    __endasm;
  }

#endif
