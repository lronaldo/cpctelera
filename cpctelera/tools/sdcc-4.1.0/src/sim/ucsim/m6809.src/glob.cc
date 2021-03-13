/*
 * Simulator of microcontrollers (glob.cc)
 *
 * Copyright (C) 2020,20 Drotos Daniel, Talker Bt.
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

#include "glob.h"

// code mask branch len mn call tick
struct dis_entry disass_m6809[]=
  {
   { 0x48, 0xff, ' ', 1, "ASLA"		, 0, 0 },
   { 0x58, 0xff, ' ', 1, "ASLB"		, 0, 0 },
   { 0x47, 0xff, ' ', 1, "ASRA"		, 0, 0 },
   { 0x57, 0xff, ' ', 1, "ASRB"		, 0, 0 },
   { 0x4f, 0xff, ' ', 1, "CLRA"		, 0, 0 },
   { 0x5f, 0xff, ' ', 1, "CLRB"		, 0, 0 },
   { 0x43, 0xff, ' ', 1, "COMA"		, 0, 0 },
   { 0x53, 0xff, ' ', 1, "COMB"		, 0, 0 },
   { 0x19, 0xff, ' ', 1, "DAA"		, 0, 0 },
   { 0x4a, 0xff, ' ', 1, "DECA"		, 0, 0 },
   { 0x5a, 0xff, ' ', 1, "DECB"		, 0, 0 },
   { 0x4c, 0xff, ' ', 1, "INCA"		, 0, 0 },
   { 0x5c, 0xff, ' ', 1, "INCB"		, 0, 0 },
   { 0x44, 0xff, ' ', 1, "LSRA"		, 0, 0 },
   { 0x54, 0xff, ' ', 1, "LSRB"		, 0, 0 },
   { 0x3d, 0xff, ' ', 1, "MUL"		, 0, 0 },
   { 0x40, 0xff, ' ', 1, "NEGA"		, 0, 0 },
   { 0x50, 0xff, ' ', 1, "NEGB"		, 0, 0 },
   { 0x12, 0xff, ' ', 1, "NOP"		, 0, 0 },
   { 0x49, 0xff, ' ', 1, "ROLA"		, 0, 0 },
   { 0x59, 0xff, ' ', 1, "ROLB"		, 0, 0 },
   { 0x46, 0xff, ' ', 1, "RORA"		, 0, 0 },
   { 0x56, 0xff, ' ', 1, "RORB"		, 0, 0 },
   { 0x3b, 0xff, ' ', 1, "RTI"		, 0, 0 },
   { 0x39, 0xff, ' ', 1, "RTS"		, 0, 0 },
   { 0x1d, 0xff, ' ', 1, "SEX"		, 0, 0 },
   { 0x3f, 0xff, ' ', 1, "SWI"		, 0, 0 },
   { 0x13, 0xff, ' ', 1, "SYNC"		, 0, 0 },
   { 0x4d, 0xff, ' ', 1, "TSTA"		, 0, 0 },
   { 0x5d, 0xff, ' ', 1, "TSTB"		, 0, 0 },

   { 0x20, 0xff, ' ', 1, "BRA %b"	, 0, 0 },
   { 0x21, 0xff, ' ', 1, "BRN %b"	, 0, 0 },
   { 0x22, 0xff, ' ', 1, "BHI %b"	, 0, 0 },
   { 0x23, 0xff, ' ', 1, "BLS %b"	, 0, 0 },
   { 0x24, 0xff, ' ', 1, "BHS %b"	, 0, 0 },
   { 0x25, 0xff, ' ', 1, "BLO %b"	, 0, 0 },
   { 0x26, 0xff, ' ', 1, "BNE %b"	, 0, 0 },
   { 0x27, 0xff, ' ', 1, "BEQ %b"	, 0, 0 },
   { 0x28, 0xff, ' ', 1, "BVC %b"	, 0, 0 },
   { 0x29, 0xff, ' ', 1, "BVS %b"	, 0, 0 },
   { 0x2a, 0xff, ' ', 1, "BPL %b"	, 0, 0 },
   { 0x2b, 0xff, ' ', 1, "BMI %b"	, 0, 0 },
   { 0x2c, 0xff, ' ', 1, "BGE %b"	, 0, 0 },
   { 0x2d, 0xff, ' ', 1, "BLT %b"	, 0, 0 },
   { 0x2e, 0xff, ' ', 1, "BGT %b"	, 0, 0 },
   { 0x2f, 0xff, ' ', 1, "BLE %b"	, 0, 0 },

   { 0x8d, 0xff, ' ', 1, "BSR %b"	, 0, 0 },
   { 0xbd, 0xff, ' ', 1, "JSR %j"	, 0, 0 },
   { 0x8d, 0xcf, ' ', 1, "JSR %b"	, 0, 0 },
   
   { 0x16, 0xff, ' ', 1, "LBRA %B"	, 0, 0 },
   { 0x17, 0xff, ' ', 1, "LBSR %B"	, 0, 0 },

   { 0x30, 0xff, ' ', 1, "LEAX %X"	, 0, 0 },
   { 0x31, 0xff, ' ', 1, "LEAY %X"	, 0, 0 },
   { 0x32, 0xff, ' ', 1, "LEAS %X"	, 0, 0 },
   { 0x33, 0xff, ' ', 1, "LEAU %X"	, 0, 0 },

   { 0x34, 0xff, ' ', 1, "PSHS %p"	, 0, 0 },
   { 0x35, 0xff, ' ', 1, "PULS %p"	, 0, 0 },
   { 0x36, 0xff, ' ', 1, "PSHU %P"	, 0, 0 },
   { 0x37, 0xff, ' ', 1, "PULU %P"	, 0, 0 },
   { 0x39, 0xff, ' ', 1, "RTS"		, 0, 0 },
   { 0x3a, 0xff, ' ', 1, "ABX"		, 0, 0 },
   { 0x3b, 0xff, ' ', 1, "RTI"		, 0, 0 },
   { 0x3d, 0xff, ' ', 1, "MUL"		, 0, 0 },
   { 0x3e, 0xff, ' ', 1, "RESET"	, 0, 0 },
   
   { 0x1c, 0xff, ' ', 1, "ANDCC %i"	, 0, 0 },
   { 0x3c, 0xff, ' ', 1, "CWAI %i"	, 0, 0 },
   { 0x1a, 0xff, ' ', 1, "ORCC %i"	, 0, 0 },

   { 0x0e, 0xff, ' ', 1, "JMP %j"	, 0, 0 },
   { 0x6e, 0xff, ' ', 1, "JMP %j"	, 0, 0 },
   { 0x7e, 0xff, ' ', 1, "JMP %J"	, 0, 0 },
   
   { 0x1e, 0xff, ' ', 1, "XCG %r"	, 0, 0 },
   { 0x1f, 0xff, ' ', 1, "TFR %r"	, 0, 0 },
   
   { 0x89, 0xcf, ' ', 1, "ADCA %u"	, 0, 0 },
   { 0xc9, 0xcf, ' ', 1, "ADCB %u"	, 0, 0 },
   { 0x8b, 0xcf, ' ', 1, "ADDA %u"	, 0, 0 },
   { 0xcb, 0xcf, ' ', 1, "ADDB %u"	, 0, 0 },
   { 0x84, 0xcf, ' ', 1, "ANDA %u"	, 0, 0 },
   { 0xc4, 0xcf, ' ', 1, "ANDB %u"	, 0, 0 },
   { 0x85, 0xcf, ' ', 1, "BITA %u"	, 0, 0 },
   { 0xc5, 0xcf, ' ', 1, "BITB %u"	, 0, 0 },
   { 0x81, 0xcf, ' ', 1, "CMPA %u"	, 0, 0 },
   { 0xc1, 0xcf, ' ', 1, "CMPB %u"	, 0, 0 },
   { 0x88, 0xcf, ' ', 1, "EORA %u"	, 0, 0 },
   { 0xc8, 0xcf, ' ', 1, "EORB %u"	, 0, 0 },
   { 0x86, 0xcf, ' ', 1, "LDA %u"	, 0, 0 },
   { 0xc6, 0xcf, ' ', 1, "LDB %u"	, 0, 0 },
   { 0x8a, 0xcf, ' ', 1, "ORA %u"	, 0, 0 },
   { 0xca, 0xcf, ' ', 1, "ORB %u"	, 0, 0 },
   { 0x82, 0xcf, ' ', 1, "SBCA %u"	, 0, 0 },
   { 0xc2, 0xcf, ' ', 1, "SBCB %u"	, 0, 0 },
   { 0x80, 0xcf, ' ', 1, "SUBA %u"	, 0, 0 },
   { 0xc0, 0xcf, ' ', 1, "SUBB %u"	, 0, 0 },

   { 0x87, 0xcf, ' ', 1, "STA %n"	, 0, 0 },
   { 0xc7, 0xcf, ' ', 1, "STB %n"	, 0, 0 },

   { 0xc3, 0xcf, ' ', 1, "ADDD %U"	, 0, 0 },
   { 0xcc, 0xcf, ' ', 1, "LDD %U"	, 0, 0 },
   { 0xce, 0xcf, ' ', 1, "LDU %U"	, 0, 0 },
   { 0x8e, 0xcf, ' ', 1, "LDX %U"	, 0, 0 },
   { 0x83, 0xcf, ' ', 1, "SUBD %U"	, 0, 0 },
   { 0x8c, 0xcf, ' ', 1, "CMPX %U"	, 0, 0 },
   
   { 0xcd, 0xcf, ' ', 1, "STD %N"	, 0, 0 },
   { 0x8f, 0xcf, ' ', 1, "STX %N"	, 0, 0 },
   { 0xcf, 0xcf, ' ', 1, "STU %N"	, 0, 0 },
   
   { 0x00, 0xcf, ' ', 1, "NEG %n"	, 0, 0 },
   { 0x60, 0x6f, ' ', 1, "NEG %n"	, 0, 0 },
   { 0x03, 0xcf, ' ', 1, "COM %n"	, 0, 0 },
   { 0x63, 0x6f, ' ', 1, "COM %n"	, 0, 0 },
   { 0x04, 0xcf, ' ', 1, "LSR %n"	, 0, 0 },
   { 0x64, 0x6f, ' ', 1, "LSR %n"	, 0, 0 },
   { 0x06, 0xcf, ' ', 1, "ROR %n"	, 0, 0 },
   { 0x66, 0x6f, ' ', 1, "ROR %n"	, 0, 0 },
   { 0x07, 0xcf, ' ', 1, "ASR %n"	, 0, 0 },
   { 0x67, 0x6f, ' ', 1, "ASR %n"	, 0, 0 },
   { 0x08, 0xcf, ' ', 1, "ASL %n"	, 0, 0 },
   { 0x68, 0x6f, ' ', 1, "ASL %n"	, 0, 0 },
   { 0x09, 0xcf, ' ', 1, "ROL %n"	, 0, 0 }, 
   { 0x69, 0x6f, ' ', 1, "ROL %n"	, 0, 0 }, 
   { 0x0a, 0xcf, ' ', 1, "DEC %n"	, 0, 0 }, 
   { 0x6a, 0x6f, ' ', 1, "DEC %n"	, 0, 0 }, 
   { 0x0c, 0xcf, ' ', 1, "INC %n"	, 0, 0 }, 
   { 0x6c, 0x6f, ' ', 1, "INC %n"	, 0, 0 }, 
   { 0x0d, 0xcf, ' ', 1, "TST %n"	, 0, 0 },
   { 0x6d, 0x6f, ' ', 1, "TST %n"	, 0, 0 },
   { 0x0f, 0xcf, ' ', 1, "CLR %n"	, 0, 0 },
   { 0x6f, 0x6f, ' ', 1, "CLR %n"	, 0, 0 },

   { 0, 0, 0, 0, 0, 0 }
  };

struct dis_entry disass_m6809_10[]=
  {
   { 0x21, 0xff, ' ', 1, "LBRN %E"	, 0, 0 },
   { 0x22, 0xff, ' ', 1, "LBHI %E"	, 0, 0 },
   { 0x23, 0xff, ' ', 1, "LBLS %E"	, 0, 0 },
   { 0x24, 0xff, ' ', 1, "LBCC %E"	, 0, 0 },
   { 0x25, 0xff, ' ', 1, "LBCS %E"	, 0, 0 },
   { 0x26, 0xff, ' ', 1, "LBNE %E"	, 0, 0 },
   { 0x27, 0xff, ' ', 1, "LBEQ %E"	, 0, 0 },
   { 0x28, 0xff, ' ', 1, "LBVC %E"	, 0, 0 },
   { 0x29, 0xff, ' ', 1, "LBVS %E"	, 0, 0 },
   { 0x2a, 0xff, ' ', 1, "LBPL %E"	, 0, 0 },
   { 0x2b, 0xff, ' ', 1, "LBMI %E"	, 0, 0 },
   { 0x2c, 0xff, ' ', 1, "LBGE %E"	, 0, 0 },
   { 0x2d, 0xff, ' ', 1, "LBLT %E"	, 0, 0 },
   { 0x2e, 0xff, ' ', 1, "LBGT %E"	, 0, 0 },
   { 0x2f, 0xff, ' ', 1, "LBLE %E"	, 0, 0 },
   { 0x83, 0xcf, ' ', 1, "CMPD %U"	, 0, 0 },
   { 0x8c, 0xcf, ' ', 1, "CMPY %U"	, 0, 0 },
   { 0xce, 0xcf, ' ', 1, "LDS %U"	, 0, 0 },
   { 0x8e, 0xcf, ' ', 1, "LDY %U"	, 0, 0 },
   { 0xcf, 0xcf, ' ', 1, "STS %U"	, 0, 0 },
   { 0x8f, 0xcf, ' ', 1, "STY %U"	, 0, 0 },
   
   { 0x3f, 0xff, ' ', 1, "SWI2"		, 0, 0 },
   
   { 0, 0, 0, 0, 0, 0 }
  };

struct dis_entry disass_m6809_11[]=
  {
   { 0x83, 0xcf, ' ', 1, "CMPU %U"	, 0, 0 },
   { 0x8c, 0xcf, ' ', 1, "CMPS %U"	, 0, 0 },
   
   { 0x3f, 0xff, ' ', 1, "SWI3"		, 0, 0 },

   { 0, 0, 0, 0, 0, 0 }
  };

/* End of m6809.src/glob.cc */
