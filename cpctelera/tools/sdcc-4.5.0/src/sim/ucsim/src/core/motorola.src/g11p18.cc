/*
 * Simulator of microcontrollers (g11p18.cc)
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

#include "g11p18.h"

// code, mask, branch, len, mnem, iscall, ticks
struct dis_entry disass11p18[]= {

  { 0x08, 0xff, ' ', 2, "INY" },
  { 0x09, 0xff, ' ', 2, "DEY" },
  { 0x30, 0xff, ' ', 2, "TSY" },
  { 0x35, 0xff, ' ', 2, "TYS" },
  { 0x38, 0xff, ' ', 2, "PULY" },
  { 0x3a, 0xff, ' ', 2, "ABY" },
  { 0x3c, 0xff, ' ', 2, "PSHY" },
  { 0x60, 0xff, ' ', 3, "NEG %x" },
  { 0x63, 0xff, ' ', 3, "COM %x" },
  { 0x64, 0xff, ' ', 3, "LSR %x" },
  { 0x66, 0xff, ' ', 3, "ROR %x" },
  { 0x67, 0xff, ' ', 3, "ASR %x" },
  { 0x68, 0xff, ' ', 3, "ASL %x" },
  { 0x69, 0xff, ' ', 3, "ROL %x" },
  { 0x6a, 0xff, ' ', 3, "DEC %x" },
  { 0x6c, 0xff, ' ', 3, "INC %x" },
  { 0x6d, 0xff, ' ', 3, "TST %x" },
  { 0x6e, 0xff, '_', 3, "JMP %x" },
  { 0x6f, 0xff, ' ', 3, "CLR %x" },
  { 0x8c, 0xff, ' ', 4, "CPY %B" },
  { 0x8f, 0xff, ' ', 2, "XGDY" },
  { 0x9c, 0xff, ' ', 3, "CPY %D" },
  { 0xa0, 0xff, ' ', 3, "SUB A,%x" },
  { 0xa1, 0xff, ' ', 3, "CMP A,%x" },
  { 0xa2, 0xff, ' ', 3, "SBC A,%x" },
  { 0xa3, 0xff, ' ', 3, "SUB D,%X" },  
  { 0xa4, 0xff, ' ', 3, "AND A,%x" },
  { 0xa5, 0xff, ' ', 3, "BIT A,%x" },
  { 0xa6, 0xff, ' ', 3, "LDA A,%x" },
  { 0xa7, 0xff, ' ', 3, "STA A,%x" },
  { 0xa8, 0xff, ' ', 3, "EOR A,%x" },
  { 0xa9, 0xff, ' ', 3, "ADC A,%x" },
  { 0xaa, 0xff, ' ', 3, "ORA A,%x" },
  { 0xab, 0xff, ' ', 3, "ADD A,%x" },
  { 0xac, 0xff, ' ', 3, "CPY %X" },
  { 0xad, 0xff, '_', 3, "JSR %X" },
  { 0xae, 0xff, ' ', 3, "LDS %X" },
  { 0xaf, 0xff, ' ', 3, "STS %X" },
  { 0xbc, 0xff, ' ', 4, "CPY %E" },
  { 0xce, 0xff, ' ', 3, "LDY %B" },
  { 0xde, 0xff, ' ', 3, "LDY %D" },
  { 0xde, 0xff, ' ', 3, "LDY %X" },
  { 0xfe, 0xff, ' ', 3, "LDY %E" },
  { 0xdf, 0xff, ' ', 3, "STY %D" },
  { 0xdf, 0xff, ' ', 3, "STY %X" },
  { 0xff, 0xff, ' ', 3, "STY %E" },
  { 0xe0, 0xff, ' ', 3, "SUB B,%x" },
  { 0xe1, 0xff, ' ', 3, "CMP B,%x" },
  { 0xe2, 0xff, ' ', 3, "SBC B,%x" },
  { 0xe3, 0xff, ' ', 3, "ADD D,%X" },
  { 0xe4, 0xff, ' ', 3, "AND B,%x" },
  { 0xe5, 0xff, ' ', 3, "BIT B,%x" },
  { 0xe6, 0xff, ' ', 3, "LDA B,%x" },
  { 0xe7, 0xff, ' ', 3, "STA B,%x" },
  { 0xe8, 0xff, ' ', 3, "EOR B,%x" },
  { 0xe9, 0xff, ' ', 3, "ADC B,%x" },
  { 0xea, 0xff, ' ', 3, "ORA B,%x" },
  { 0xeb, 0xff, ' ', 3, "ADD B,%x" },
  { 0xec, 0xff, ' ', 3, "LDD %X" },
  { 0xed, 0xff, ' ', 3, "STD %X" },

  { 0x1c, 0xff, ' ', 3, "BSET %x,%b" },
  { 0x1e, 0xff, 'R', 4, "BRSET %x,%b,%r" },
  { 0x1d, 0xff, ' ', 3, "BCLR %x,%b" },
  { 0x1f, 0xff, 'R', 4, "BRCLR %x,%b,%r" },

  { 0, 0, 0, 0, 0, 0 }
};

/* End of m6800.src/g11p18.cc */
