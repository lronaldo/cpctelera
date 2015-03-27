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
  TYPE_UBYTE h;
  TYPE_UBYTE l;
#else
  TYPE_UBYTE l;
  TYPE_UBYTE h;
#endif
};

#define DEF_REGPAIR(BIGNAME,smallname) \
  union { \
    TYPE_UWORD BIGNAME; \
    struct t_regpair smallname; \
  }

struct t_regs
{
  TYPE_UBYTE A;
  TYPE_UBYTE F;
  DEF_REGPAIR(BC, bc);
  DEF_REGPAIR(DE, de);
  DEF_REGPAIR(HL, hl);
  DEF_REGPAIR(IX, ix);
  DEF_REGPAIR(IY, iy);
  TYPE_UWORD SP;
  /* there are alternate AF,BC,DE,HL register sets, and a few instructions
     that swap one for the other */
  TYPE_UBYTE aA;
  TYPE_UBYTE aF;
  DEF_REGPAIR(aBC, a_bc);
  DEF_REGPAIR(aDE, a_de);
  DEF_REGPAIR(aHL, a_hl);
  
  TYPE_UBYTE iv;  /* interrupt vector, see ed 47 ld A,IV.. */
};

#define BIT_C	0x01  // carry status(out of bit 7)
#define BIT_N	0x02  // Not addition: subtract status(1 after subtract).
#define BIT_P	0x04  // parity/overflow, 1=even, 0=odd parity.  arith:1=overflow
#define BIT_A	0x10  // aux carry status(out of bit 3)
#define BIT_Z	0x40  // zero status, 1=zero, 0=nonzero
#define BIT_S	0x80  // sign status(value of bit 7)
#define BIT_ALL	(BIT_C |BIT_N |BIT_P |BIT_A |BIT_Z |BIT_S)  // all bits

#define BITPOS_C 0    // 1
#define BITPOS_SUB 1  // 2H
#define BITPOS_P 2    // 4H
#define BITPOS_A 4    // 10H
#define BITPOS_Z 6    // 40H
#define BITPOS_S 7    // 80H

#endif

/* End of z80.src/regsz80.h */
