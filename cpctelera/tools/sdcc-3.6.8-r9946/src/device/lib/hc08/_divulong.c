/*-------------------------------------------------------------------------
   _divulong.c - routine for division of 32 bit unsigned long

   Copyright (C) 1999, Jean-Louis Vern <jlvern AT gmail.com>

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

/*   Assembler-functions are provided for:
     mcs51 small
     mcs51 small stack-auto
*/

#if !defined(__SDCC_USE_XSTACK) && !defined(_SDCC_NO_ASM_LIB_FUNCS)
#  if defined(__SDCC_mcs51)
#    if defined(__SDCC_MODEL_SMALL)
#      if defined(__SDCC_STACK_AUTO) && !defined(__SDCC_PARMS_IN_BANK1)
#        define _DIVULONG_ASM_SMALL_AUTO
#      else
#        define _DIVULONG_ASM_SMALL
#      endif
#    endif
#  endif
#endif

#if defined _DIVULONG_ASM_SMALL

static void
_divlong_dummy (void) _naked
{
  __asm

    .globl __divulong

  __divulong:

    #define count   r2

    #define a0  dpl
    #define a1  dph
    #define a2  b
    #define a3  r3

    #define reste0  r4
    #define reste1  r5
    #define reste2  r6
    #define reste3  r7
#if !defined(__SDCC_PARMS_IN_BANK1)

#if defined(__SDCC_NOOVERLAY)
    .area DSEG    (DATA)
#else
    .area OSEG    (OVR,DATA)
#endif

    .globl __divulong_PARM_2
    .globl __divslong_PARM_2

  __divulong_PARM_2:
  __divslong_PARM_2:
    .ds 4

    .area CSEG    (CODE)

    #define b0      (__divulong_PARM_2)
    #define b1      (__divulong_PARM_2 + 1)
    #define b2      (__divulong_PARM_2 + 2)
    #define b3      (__divulong_PARM_2 + 3)
#else
    #define b0      (b1_0)
    #define b1      (b1_1)
    #define b2      (b1_2)
    #define b3      (b1_3)
#endif // !__SDCC_PARMS_IN_BANK1
          ; parameter a comes in a, b, dph, dpl
    mov a3,a    ; save parameter a3

    mov count,#32
    clr a
    mov reste0,a
    mov reste1,a
    mov reste2,a
    mov reste3,a

  ; optimization  loop in lp0 until the first bit is shifted into rest

  lp0:  mov a,a0    ; a <<= 1
    add a,a0
    mov a0,a
    mov a,a1
    rlc a
    mov a1,a
    mov a,a2
    rlc a
    mov a2,a
    mov a,a3
    rlc a
    mov a3,a

    jc  in_lp
    djnz  count,lp0

    sjmp  exit

  loop: mov a,a0    ; a <<= 1
    add a,a0
    mov a0,a
    mov a,a1
    rlc a
    mov a1,a
    mov a,a2
    rlc a
    mov a2,a
    mov a,a3
    rlc a
    mov a3,a

  in_lp:  mov a,reste0  ; reste <<= 1
    rlc a   ;   feed in carry
    mov reste0,a
    mov a,reste1
    rlc a
    mov reste1,a
    mov a,reste2
    rlc a
    mov reste2,a
    mov a,reste3
    rlc a
    mov reste3,a

    mov a,reste0  ; reste - b
    subb  a,b0    ; carry is always clear here, because
                  ; reste <<= 1 never overflows
    mov a,reste1
    subb  a,b1
    mov a,reste2
    subb  a,b2
    mov a,reste3
    subb  a,b3

    jc  minus   ; reste >= b?

          ; -> yes;  reste -= b;
    mov a,reste0
    subb  a,b0    ; carry is always clear here (jc)
    mov reste0,a
    mov a,reste1
    subb  a,b1
    mov reste1,a
    mov a,reste2
    subb  a,b2
    mov reste2,a
    mov a,reste3
    subb  a,b3
    mov reste3,a

    orl a0,#1

  minus:  djnz  count,loop  ; -> no

  exit: mov a,a3    ; prepare the return value
    ret

  __endasm ;
}

#elif defined _DIVULONG_ASM_SMALL_AUTO

static void
_divlong_dummy (void) _naked
{
  __asm

    .globl __divulong

  __divulong:

    #define count   r2

    #define a0  dpl
    #define a1  dph
    #define a2  b
    #define a3  r3

    #define reste0  r4
    #define reste1  r5
    #define reste2  r6
    #define reste3  r7

    .globl __divlong  ; entry point for __divslong

    #define b0  r1

    ar0 = 0     ; BUG register set is not considered
    ar1 = 1

          ; parameter a comes in a, b, dph, dpl
    mov a3,a    ; save parameter a3

    mov a,sp
    add a,#-2-3   ; 2 bytes return address, 3 bytes param b
    mov r0,a    ; r0 points to b0

  __divlong:      ; entry point for __divslong

    mov ar1,@r0   ; load b0
    inc r0    ; r0 points to b1

    mov count,#32
    clr a
    mov reste0,a
    mov reste1,a
    mov reste2,a
    mov reste3,a

  ; optimization  loop in lp0 until the first bit is shifted into rest

  lp0:  mov a,a0    ; a <<= 1
    add a,a0
    mov a0,a
    mov a,a1
    rlc a
    mov a1,a
    mov a,a2
    rlc a
    mov a2,a
    mov a,a3
    rlc a
    mov a3,a

    jc  in_lp
    djnz  count,lp0

    sjmp  exit

  loop: mov a,a0    ; a <<= 1
    add a,a0
    mov a0,a
    mov a,a1
    rlc a
    mov a1,a
    mov a,a2
    rlc a
    mov a2,a
    mov a,a3
    rlc a
    mov a3,a

  in_lp:  mov a,reste0  ; reste <<= 1
    rlc a   ;   feed in carry
    mov reste0,a
    mov a,reste1
    rlc a
    mov reste1,a
    mov a,reste2
    rlc a
    mov reste2,a
    mov a,reste3
    rlc a
    mov reste3,a

    mov a,reste0  ; reste - b
    subb  a,b0    ; carry is always clear here, because
          ; reste <<= 1 never overflows
    mov a,reste1
    subb  a,@r0   ; b1
    mov a,reste2
    inc r0
    subb  a,@r0   ; b2
    mov a,reste3
    inc r0
    subb  a,@r0   ; b3
    dec r0
    dec r0

    jc  minus   ; reste >= b?

          ; -> yes;  reste -= b;
    mov a,reste0
    subb  a,b0    ; carry is always clear here (jc)
    mov reste0,a
    mov a,reste1
    subb  a,@r0   ; b1
    mov reste1,a
    mov a,reste2
    inc r0
    subb  a,@r0   ; b2
    mov reste2,a
    mov a,reste3
    inc r0
    subb  a,@r0   ; b3
    mov reste3,a
    dec r0
    dec r0

    orl a0,#1

  minus:  djnz  count,loop  ; -> no

  exit: mov a,a3    ; prepare the return value
    ret

  __endasm ;
}

#else // _DIVULONG_ASM

#define MSB_SET(x) ((x >> (8*sizeof(x)-1)) & 1)

unsigned long
_divulong (unsigned long a, unsigned long b)
{
  unsigned long reste = 0L;
  unsigned char count = 32;
  char c;

  do
  {
    // reste: a <- 0;
    c = MSB_SET(a);
    a <<= 1;
    reste <<= 1;
    if (c)
      reste |= 1L;

    if (reste >= b)
    {
      reste -= b;
      // a <- (result = 1)
      a |= 1L;
    }
  }
  while (--count);
  return a;
}

#endif // _DIVULONG_ASM
