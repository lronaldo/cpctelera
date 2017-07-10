/*
 * Simulator of microcontrollers (regsz80.h)
 *
 * some z80 code base from Karl Bongers karl@turbobit.com
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
 * 
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
 *
 */

/* This file is part of microcontroller simulator: ucsim.

UCSIM is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

UCSIM is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with UCSIM; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA. */
/*@1@*/

#ifndef REGSZ80_HEADER
#define REGSZ80_HEADER

#include "ddconfig.h"


struct t_regpair
{
#ifdef WORDS_BIGENDIAN
  u8_t/*TYPE_UBYTE*/ h;
  u8_t/*TYPE_UBYTE*/ l;
#else
  u8_t/*TYPE_UBYTE*/ l;
  u8_t/*TYPE_UBYTE*/ h;
#endif
};

#define DEF_REGPAIR(BIGNAME,smallname) \
  union { \
    u16_t/*TYPE_UWORD*/ BIGNAME;		\
    struct t_regpair smallname; \
  }

struct t_regs
{
  //TYPE_UBYTE A;
  //TYPE_UBYTE F;
  union {
    u16_t AF;
    struct {
#ifdef WORDS_BIGENDIAN
      u8_t/*TYPE_UBYTE*/ A;
      u8_t/*TYPE_UBYTE*/ F;
#else
      u8_t/*TYPE_UBYTE*/ F;
      u8_t/*TYPE_UBYTE*/ A;
#endif
    } raf;
  };
  DEF_REGPAIR(BC, bc);
  DEF_REGPAIR(DE, de);
  DEF_REGPAIR(HL, hl);
  DEF_REGPAIR(IX, ix);
  DEF_REGPAIR(IY, iy);
  u16_t/*TYPE_UWORD*/ SP;
  /* there are alternate AF,BC,DE,HL register sets, and a few instructions
     that swap one for the other */
  //TYPE_UBYTE aA;
  //TYPE_UBYTE aF;
  union {
    u16_t aAF;
    struct {
#ifdef WORDS_BIGENDIAN
      u8_t/*TYPE_UBYTE*/ aA;
      u8_t/*TYPE_UBYTE*/ aF;
#else
      u8_t/*TYPE_UBYTE*/ aF;
      u8_t/*TYPE_UBYTE*/ aA;
#endif
    } ralt_af;
  };
  DEF_REGPAIR(aBC, a_bc);
  DEF_REGPAIR(aDE, a_de);
  DEF_REGPAIR(aHL, a_hl);
  
  u8_t/*TYPE_UBYTE*/ iv;  /* interrupt vector, see ed 47 ld A,IV.. */
};

enum {
  BIT_C=	0x01,  // carry status(out of bit 7)
  BIT_N=	0x02,  // Not addition: subtract status(1 after subtract).
  BIT_P=	0x04,  // parity/overflow, 1=even, 0=odd parity.  arith:1=overflow
  BIT_A=	0x10,  // aux carry status(out of bit 3)
  BIT_Z=	0x40,  // zero status, 1=zero, 0=nonzero
  BIT_S=	0x80,  // sign status(value of bit 7)
  BIT_ALL=	(BIT_C |BIT_N |BIT_P |BIT_A |BIT_Z |BIT_S)  // all bits
};

enum {
  BITPOS_C= 0,    // 1
  BITPOS_SUB= 1,  // 2H
  BITPOS_P= 2,    // 4H
  BITPOS_A= 4,    // 10H
  BITPOS_Z= 6,    // 40H
  BITPOS_S= 7    // 80H
};

#endif

/* End of z80.src/regsz80.h */
