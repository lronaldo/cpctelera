/*
 * Simulator of microcontrollers (glob.cc)
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

//#include <stdint.h>

#include "glob.h"


instruction_wrapper_fn itab[256];
instruction_wrapper_fn itab_dd[256];
instruction_wrapper_fn itab_ed[256];
instruction_wrapper_fn itab_fd[256];
instruction_wrapper_fn itab_7f[256];

u8_t sbox_tab[256];
u8_t ibox_tab[256];

/* 
%d - signed compl.,byte jump 
%w - 2-byte jump or imm. value
%b - byte imm. value
*/
// code mask branch len mn call tick
struct dis_entry disass_rxk[]=
  {
    { 0x76, 0xff, ' ', 1, "ALTD" },
    { 0xd3, 0xff, ' ', 1, "IOI" },
    { 0xdb, 0xff, ' ', 1, "IOE" },

    { 0x00, 0xff, ' ', 1, "NOP" },
    { 0x01, 0xff, ' ', 3, "LD BC,%w" },
    { 0x11, 0xff, ' ', 3, "LD DE,%w" },
    { 0x21, 0xff, ' ', 3, "LD HL,%w" },
    { 0x31, 0xff, ' ', 3, "LD SP,%w" },
    { 0x22, 0xff, ' ', 3, "LD (%w),HL" },
    { 0x2a, 0xff, ' ', 3, "LD HL,(%w)" },
    { 0x03, 0xff, ' ', 1, "INC BC" },
    { 0x13, 0xff, ' ', 1, "INC DE" },
    { 0x23, 0xff, ' ', 1, "INC HL" },
    { 0x33, 0xff, ' ', 1, "INC SP" },
    { 0x3c, 0xff, ' ', 1, "INC A" },
    { 0x04, 0xff, ' ', 1, "INC B" },
    { 0x0c, 0xff, ' ', 1, "INC C" },
    { 0x14, 0xff, ' ', 1, "INC D" },
    { 0x1c, 0xff, ' ', 1, "INC E" },
    { 0x24, 0xff, ' ', 1, "INC H" },
    { 0x2c, 0xff, ' ', 1, "INC L" },
    { 0x0b, 0xff, ' ', 1, "DEC BC" },
    { 0x1b, 0xff, ' ', 1, "DEC DE" },
    { 0x2b, 0xff, ' ', 1, "DEC HL" },
    { 0x3b, 0xff, ' ', 1, "DEC SP" },
    { 0x3d, 0xff, ' ', 1, "DEC A" },
    { 0x05, 0xff, ' ', 1, "DEC B" },
    { 0x0d, 0xff, ' ', 1, "DEC C" },
    { 0x15, 0xff, ' ', 1, "DEC D" },
    { 0x1d, 0xff, ' ', 1, "DEC E" },
    { 0x25, 0xff, ' ', 1, "DEC H" },
    { 0x2d, 0xff, ' ', 1, "DEC L" },
    { 0x3e, 0xff, ' ', 2, "LD A,%b" },
    { 0x06, 0xff, ' ', 2, "LD B,%b" },
    { 0x0e, 0xff, ' ', 2, "LD C,%b" },
    { 0x16, 0xff, ' ', 2, "LD D,%b" },
    { 0x1e, 0xff, ' ', 2, "LD E,%b" },
    { 0x26, 0xff, ' ', 2, "LD H,%b" },
    { 0x2e, 0xff, ' ', 2, "LD L,%b" },
    { 0x07, 0xff, ' ', 1, "RLCA" },
    { 0x17, 0xff, ' ', 1, "RLA" },
    { 0x0f, 0xff, ' ', 1, "RRCA" },
    { 0x1f, 0xff, ' ', 1, "RRA" },
    { 0x02, 0xff, ' ', 1, "LD (BC),A" },
    { 0x12, 0xff, ' ', 1, "LD (DE),A" },
    { 0x77, 0xff, ' ', 1, "LD (HL),A" },
    { 0x70, 0xff, ' ', 1, "LD (HL),B" },
    { 0x71, 0xff, ' ', 1, "LD (HL),C" },
    { 0x72, 0xff, ' ', 1, "LD (HL),D" },
    { 0x73, 0xff, ' ', 1, "LD (HL),E" },
    { 0x74, 0xff, ' ', 1, "LD (HL),H" },
    { 0x75, 0xff, ' ', 1, "LD (HL),L" },
    { 0x32, 0xff, ' ', 3, "LD (%w),A" },
    { 0x0a, 0xff, ' ', 1, "LD A,(BC)" },
    { 0x1a, 0xff, ' ', 1, "LD A,(DE)" },
    { 0x3a, 0xff, ' ', 3, "LD A,(%w)" },
    { 0x7e, 0xff, ' ', 1, "LD A,(HL)" },
    { 0x46, 0xff, ' ', 1, "LD B,(HL)" },
    { 0x4e, 0xff, ' ', 1, "LD C,(HL)" },
    { 0x56, 0xff, ' ', 1, "LD D,(HL)" },
    { 0x5e, 0xff, ' ', 1, "LD E,(HL)" },
    { 0x66, 0xff, ' ', 1, "LD H,(HL)" },
    { 0x6e, 0xff, ' ', 1, "LD L,(HL)" },
    { 0x37, 0xff, ' ', 1, "SCF" },
    { 0x2f, 0xff, ' ', 1, "CPL" },
    { 0x3f, 0xff, ' ', 1, "CCF" },
    { 0x08, 0xff, ' ', 1, "EX AF,AF'" }, // '
    { 0x09, 0xff, ' ', 1, "ADD HL,BC" },
    { 0x19, 0xff, ' ', 1, "ADD HL,DE" },
    { 0x29, 0xff, ' ', 1, "ADD HL,HL" },
    { 0x39, 0xff, ' ', 1, "ADD HL,SP" },
    { 0x10, 0xff, ' ', 2, "DJNZ %r" },
    { 0x18, 0xff, ' ', 2, "JR %r" },
    { 0x20, 0xff, ' ', 2, "JR NZ,%r" },
    { 0x28, 0xff, ' ', 2, "JR Z,%r" },
    { 0x30, 0xff, ' ', 2, "JR NC,%r" },
    { 0x38, 0xff, ' ', 2, "JR C,%r" },
    { 0x27, 0xff, ' ', 2, "ADD SP,%b" },
    { 0x34, 0xff, ' ', 1, "INC (HL)" },
    { 0x35, 0xff, ' ', 1, "DEC (HL)" },
    { 0x36, 0xff, ' ', 2, "LD (HL),%b" },
    { 0x47, 0xff, ' ', 1, "LD B,A" },
    { 0x4f, 0xff, ' ', 1, "LD C,A" },
    { 0x57, 0xff, ' ', 1, "LD D,A" },
    { 0x5b, 0xff, ' ', 1, "LD E,E" },
    { 0x5f, 0xff, ' ', 1, "LD E,A" },
    { 0x6f, 0xff, ' ', 1, "LD L,A" },
    { 0x6d, 0xff, ' ', 1, "LD L,L" },
    { 0x67, 0xff, ' ', 1, "LD H,A" },
    { 0x78, 0xff, ' ', 1, "LD A,B" },
    { 0x79, 0xff, ' ', 1, "LD A,C" },
    { 0x7a, 0xff, ' ', 1, "LD A,D" },
    { 0x7b, 0xff, ' ', 1, "LD A,E" },
    { 0x7c, 0xff, ' ', 1, "LD A,H" },
    { 0x7d, 0xff, ' ', 1, "LD A,L" },
    { 0x7f, 0xff, ' ', 1, "LD A,A" },
    { 0xaf, 0xff, ' ', 1, "XOR A" },
    { 0xb7, 0xff, ' ', 1, "OR A" },
    { 0xc0, 0xff, ' ', 1, "RET NZ" },
    { 0xc8, 0xff, ' ', 1, "RET Z" },
    { 0xc9, 0xff, ' ', 1, "RET" },
    { 0xd0, 0xff, ' ', 1, "RET NC" },
    { 0xd8, 0xff, ' ', 1, "RET C" },
    { 0xe0, 0xff, ' ', 1, "RET LZ" },
    { 0xe8, 0xff, ' ', 1, "RET LO" },
    { 0xf0, 0xff, ' ', 1, "RET P" },
    { 0xf8, 0xff, ' ', 1, "RET M" },
    { 0xf1, 0xff, ' ', 1, "POP AF" },
    { 0xc1, 0xff, ' ', 1, "POP BC" },
    { 0xd1, 0xff, ' ', 1, "POP DE" },
    { 0xe1, 0xff, ' ', 1, "POP HL" },
    { 0xc2, 0xff, ' ', 3, "JP NZ,%w" },
    { 0xca, 0xff, ' ', 3, "JP Z,%w" },
    { 0xd2, 0xff, ' ', 3, "JP NC,%w" },
    { 0xda, 0xff, ' ', 3, "JP C,%w" },
    { 0xe2, 0xff, ' ', 3, "JP LZ,%w" },
    { 0xea, 0xff, ' ', 3, "JP LO,%w" },
    { 0xf2, 0xff, ' ', 3, "JP P,%w" },
    { 0xfa, 0xff, ' ', 3, "JP M,%w" },
    { 0xc3, 0xff, ' ', 3, "JP %w" },
    { 0xc4, 0xff, ' ', 2, "LD HL,(SP+%b)" },
    { 0xf5, 0xff, ' ', 1, "PUSH AF" },
    { 0xc5, 0xff, ' ', 1, "PUSH BC" },
    { 0xd5, 0xff, ' ', 1, "PUSH DE" },
    { 0xe5, 0xff, ' ', 1, "PUSH HL" },
    { 0xc6, 0xff, ' ', 2, "ADD A,%b" },
    { 0xc7, 0xff, ' ', 4, "LJP %l" },
    { 0xcc, 0xff, ' ', 1, "BOOL HL" },
    { 0xcd, 0xff, ' ', 3, "CALL %w" },
    { 0xce, 0xff, ' ', 2, "ADC A,%b" },
    { 0xcf, 0xff, ' ', 4, "LCALL %l" },
    { 0xd4, 0xff, ' ', 2, "LD (SP+%b),HL" },
    { 0xd6, 0xff, ' ', 2, "SUB A,%b" },
    { 0xd7, 0xff, ' ', 1, "RST 10" },
    { 0xdf, 0xff, ' ', 1, "RST 18" },
    { 0xe7, 0xff, ' ', 1, "RST 20" },
    { 0xef, 0xff, ' ', 1, "RST 28" },
    { 0xff, 0xff, ' ', 1, "RST 38" },
    { 0xd9, 0xff, ' ', 1, "EXX" },
    { 0xdc, 0xff, ' ', 1, "AND HL,DE" },
    { 0xec, 0xff, ' ', 1, "OR HL,DE" },
    { 0xde, 0xff, ' ', 2, "SBC A,%b" },
    { 0xe3, 0xff, ' ', 1, "EX DE',HL" }, // '
    { 0xeb, 0xff, ' ', 1, "EX DE,HL" },
    { 0xe4, 0xff, ' ', 2, "LD HL,(IX%d)" },
    { 0xf4, 0xff, ' ', 2, "LD (IX%d),HL" },
    { 0xe6, 0xff, ' ', 2, "AND A,%b" },
    { 0xe9, 0xff, ' ', 1, "JP HL" },
    { 0xee, 0xff, ' ', 2, "XOR %b" },
    { 0xf3, 0xff, ' ', 1, "RL DE" },
    { 0xf6, 0xff, ' ', 2, "OR %b" },
    { 0xf7, 0xff, ' ', 1, "MUL" },
    { 0xf9, 0xff, ' ', 1, "LD SP,HL" },
    { 0xfb, 0xff, ' ', 1, "RR DE" },
    { 0xfc, 0xff, ' ', 1, "RR HL" },
    { 0xfe, 0xff, ' ', 2, "CP A,%b" },
    
    { 0, 0, 0, 0, 0, 0, 0 }
  };


#define ROTL8(x,shift) ((/*u8_t*/u8_t) ((x) << (shift)) | ((x) >> (8 - (shift))))

void init_sbox()
{
  /*u8_t*/u8_t p = 1, q = 1;
	
  /* loop invariant: p * q == 1 in the Galois field */
  do
    {
      /* multiply p by 3 */
      p = p ^ (p << 1) ^ (p & 0x80 ? 0x1B : 0);
      
      /* divide q by 3 (equals multiplication by 0xf6) */
      q ^= q << 1;
      q ^= q << 2;
      q ^= q << 4;
      q ^= q & 0x80 ? 0x09 : 0;
      
      /* compute the affine transformation */
      /*u8_t*/u8_t xformed = q ^ ROTL8(q, 1) ^ ROTL8(q, 2) ^ ROTL8(q, 3) ^ ROTL8(q, 4);
      u8_t val= xformed ^ 0x63;
      sbox_tab[p] = val;
      ibox_tab[val]= p;
    }
  while (p != 1);
  
  /* 0 is a special case since it has no inverse */
  sbox_tab[0] = 0x63;
  ibox_tab[0x63]= 0;
}

/* End of rxk.src/glob.cc */
