/*
 * Simulator of microcontrollers (regsstm8.h)
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

/* $Id: regsstm8.h 496 2016-11-11 12:48:27Z drdani $ */

#ifndef REGSTM8_HEADER
#define REGSTM8_HEADER

#include "ddconfig.h"


struct t_regs
{
  u8_t A;
  u16_t X;
  u16_t Y;
  u16_t SP;
  u8_t CC;
  //u8_t VECTOR;
};

#define BIT_C	0x01  // carry status(out of bit 7)
#define BIT_Z	0x02  // zero status, 1=zero, 0=nonzero
#define BIT_N	0x04  // sign, 1=negative, 0=positive (or zero)
#define BIT_I0	0x08  // interrupt mask, 1=disabled, 0=enabled
#define BIT_H	0x10  // half carry status(out of bit 3)
#define BIT_I1	0x20  // interrupt mask, 1=disabled, 0=enabled
#define BIT_V	0x80  // signed overflow, 1=overflow
#define BIT_ALL	(BIT_C |BIT_Z |BIT_N |BIT_I0 |BIT_H |BIT_I1 |BIT_V)  // all bits

#define BITPOS_C 0    // 1
#define BITPOS_Z 1    // 2H
#define BITPOS_N 2    // 4H
#define BITPOS_I0 3    // 8H
#define BITPOS_H 4    // 10H
#define BITPOS_I1 5    // 20H
#define BITPOS_V 7    // 80H

#endif

/* End of stm8.src/regsstm8.h */
