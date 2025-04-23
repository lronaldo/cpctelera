/*
 * Simulator of microcontrollers (g11p0.cc)
 *
 * Copyright (C) 2020 Drotos Daniel
 * 
 * To contact author send email to dr.dkdb@gmail.com
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

#include <stdio.h>

#include "g11p0.h"

// code, mask, branch, len, mnem, iscall, ticks
struct dis_entry disass11p0[]= {

  { 0x00, 0xff, ' ', 1, "TEST" },
  { 0x02, 0xff, ' ', 1, "IDIV" },
  { 0x03, 0xff, ' ', 1, "FDIV" },
  { 0x04, 0xff, ' ', 1, "LSRD" },
  { 0x05, 0xff, ' ', 1, "ASL D" },
  { 0x21, 0xff, 'R', 2, "BRN %r" },
  { 0x38, 0xff, ' ', 1, "PULX" },
  { 0x3a, 0xff, ' ', 1, "ABX" },
  { 0x3c, 0xff, ' ', 1, "PSHX" },
  { 0x3d, 0xff, ' ', 1, "MUL" },
  { 0x83, 0xff, ' ', 3, "SUB D,%B" },
  { 0x93, 0xff, ' ', 2, "SUB D,%D" },
  { 0xa3, 0xff, ' ', 2, "SUB D,%X" },
  { 0xb3, 0xff, ' ', 3, "SUB D,%E" },
  { 0x8f, 0xff, ' ', 1, "XGDX" },
  { 0x9d, 0xff, 'd', 2, "JSR %d" },
  { 0xc3, 0xff, ' ', 3, "ADD D,%B" },
  { 0xd3, 0xff, ' ', 2, "ADD D,%D" },
  { 0xe3, 0xff, ' ', 2, "ADD D,%X" },
  { 0xf3, 0xff, ' ', 3, "ADD D,%E" },
  { 0xcc, 0xff, ' ', 3, "LDD %B" },
  { 0xdc, 0xff, ' ', 2, "LDD %D" },
  { 0xec, 0xff, ' ', 2, "LDD %X" },
  { 0xfc, 0xff, ' ', 3, "LDD %E" },
  { 0xcf, 0xff, ' ', 1, "STOP" },
  { 0xdd, 0xff, ' ', 2, "STD %D" },
  { 0xed, 0xff, ' ', 2, "STD %X" },
  { 0xfd, 0xff, ' ', 3, "STD %E" },
  { 0x12, 0xff, 'R', 4, "BRSET %d,%b,%r" },
  { 0x1e, 0xff, 'R', 4, "BRSET %x,%b,%r" },
  { 0x13, 0xff, 'R', 4, "BRCLR %d,%b,%r" },
  { 0x1f, 0xff, 'R', 4, "BRCLR %x,%b,%r" },
  { 0x14, 0xff, ' ', 3, "BSET %d,%b" },
  { 0x1c, 0xff, ' ', 3, "BSET %x,%b" },
  { 0x15, 0xff, ' ', 3, "BCLR %d,%b" },
  { 0x1d, 0xff, ' ', 3, "BCLR %x,%b" },
  
  { 0, 0, 0, 0, 0, 0 }
};

/* End of m6800.src/g11p0.cc */
