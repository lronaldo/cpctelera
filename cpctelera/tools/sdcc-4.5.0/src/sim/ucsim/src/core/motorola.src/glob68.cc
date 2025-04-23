/*
 * Simulator of microcontrollers (glob68.cc)
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

#include "glob68.h"


// code mask branch len mn call tick
struct dis_entry disass_m6800[]=
  {
    { 0x01, 0xff, ' ', 1, "NOP" },
    { 0x06, 0xff, ' ', 1, "TAP" },
    { 0x07, 0xff, ' ', 1, "TAP" },
    { 0x08, 0xff, ' ', 1, "INX" },
    { 0x09, 0xff, ' ', 1, "DEX" },
    { 0x0a, 0xff, ' ', 1, "CLV" },
    { 0x0b, 0xff, ' ', 1, "SEV" },
    { 0x0c, 0xff, ' ', 1, "CLC" },
    { 0x0d, 0xff, ' ', 1, "SEC" },
    { 0x0e, 0xff, ' ', 1, "CLI" },
    { 0x0f, 0xff, ' ', 1, "SEI" },

    { 0x10, 0xff, ' ', 1, "SBA" },
    { 0x11, 0xff, ' ', 1, "CBA" },
    { 0x16, 0xff, ' ', 1, "TAB" },
    { 0x17, 0xff, ' ', 1, "TBA" },
    { 0x19, 0xff, ' ', 1, "DAA" },
    { 0x1b, 0xff, ' ', 1, "ABA" },

    { 0x20, 0xff, 'r', 2, "BRA %r" },
    { 0x22, 0xff, 'R', 2, "BHI %r" },
    { 0x23, 0xff, 'R', 2, "BLS %r" },
    { 0x24, 0xff, 'R', 2, "BCC %r" },
    { 0x25, 0xff, 'R', 2, "BCS %r" },
    { 0x26, 0xff, 'R', 2, "BNE %r" },
    { 0x27, 0xff, 'R', 2, "BEQ %r" },
    { 0x28, 0xff, 'R', 2, "BVC %r" },
    { 0x29, 0xff, 'R', 2, "BVS %r" },
    { 0x2a, 0xff, 'R', 2, "BPL %r" },
    { 0x2b, 0xff, 'R', 2, "BMI %r" },
    { 0x2c, 0xff, 'R', 2, "BGE %r" },
    { 0x2d, 0xff, 'R', 2, "BLT %r" },
    { 0x2e, 0xff, 'R', 2, "BGT %r" },
    { 0x2f, 0xff, 'R', 2, "BLE %r" },

    { 0x30, 0xff, ' ', 1, "TSX" },
    { 0x31, 0xff, ' ', 1, "INS" },
    { 0x32, 0xff, ' ', 1, "PUL A" },
    { 0x33, 0xff, ' ', 1, "PUL B" },
    { 0x34, 0xff, ' ', 1, "DES" },
    { 0x35, 0xff, ' ', 1, "TXS" },
    { 0x36, 0xff, ' ', 1, "PSH A" },
    { 0x37, 0xff, ' ', 1, "PSH B" },
    { 0x39, 0xff, '_', 1, "RTS" },

    { 0x3b, 0xff, '_', 1, "RTI" },
    { 0x3e, 0xff, ' ', 1, "WAI" },
    { 0x3f, 0xff, 's', 1, "SWI" },

    { 0x40, 0xff, ' ', 1, "NEG A" },
    { 0x43, 0xff, ' ', 1, "COM A" },
    { 0x44, 0xff, ' ', 1, "LSR A" },
    { 0x46, 0xff, ' ', 1, "ROR A" },
    { 0x47, 0xff, ' ', 1, "ASR A" },
    { 0x48, 0xff, ' ', 1, "ASL A" },
    { 0x49, 0xff, ' ', 1, "ROL A" },
    { 0x4a, 0xff, ' ', 1, "DEC A" },
    { 0x4c, 0xff, ' ', 1, "INC A" },
    { 0x4d, 0xff, ' ', 1, "TST A" },
    { 0x4f, 0xff, ' ', 1, "CLR A" },
    
    { 0x50, 0xff, ' ', 1, "NEG B" },
    { 0x53, 0xff, ' ', 1, "COM B" },
    { 0x54, 0xff, ' ', 1, "LSR B" },
    { 0x56, 0xff, ' ', 1, "ROR B" },
    { 0x57, 0xff, ' ', 1, "ASR B" },
    { 0x58, 0xff, ' ', 1, "ASL B" },
    { 0x59, 0xff, ' ', 1, "ROL B" },
    { 0x5a, 0xff, ' ', 1, "DEC B" },
    { 0x5c, 0xff, ' ', 1, "INC B" },
    { 0x5d, 0xff, ' ', 1, "TST B" },
    { 0x5f, 0xff, ' ', 1, "CLR B" },
    
    { 0x60, 0xff, ' ', 2, "NEG %x" },
    { 0x63, 0xff, ' ', 2, "COM %x" },
    { 0x64, 0xff, ' ', 2, "LSR %x" },
    { 0x66, 0xff, ' ', 2, "ROR %x" },
    { 0x67, 0xff, ' ', 2, "ASR %x" },
    { 0x68, 0xff, ' ', 2, "ASL %x" },
    { 0x69, 0xff, ' ', 2, "ROL %x" },
    { 0x6a, 0xff, ' ', 2, "DEC %x" },
    { 0x6c, 0xff, ' ', 2, "INC %x" },
    { 0x6d, 0xff, ' ', 2, "TST %x" },
    { 0x6e, 0xff, '_', 2, "JMP %x" },
    { 0x6f, 0xff, ' ', 2, "CLR %x" },
    
    { 0x70, 0xff, ' ', 3, "NEG %e" },
    { 0x73, 0xff, ' ', 3, "COM %e" },
    { 0x74, 0xff, ' ', 3, "LSR %e" },
    { 0x76, 0xff, ' ', 3, "ROR %e" },
    { 0x77, 0xff, ' ', 3, "ASR %e" },
    { 0x78, 0xff, ' ', 3, "ASL %e" },
    { 0x79, 0xff, ' ', 3, "ROL %e" },
    { 0x7a, 0xff, ' ', 3, "DEC %e" },
    { 0x7c, 0xff, ' ', 3, "INC %e" },
    { 0x7d, 0xff, ' ', 3, "TST %e" },
    { 0x7e, 0xff, 'e', 3, "JMP %e" },
    { 0x7f, 0xff, ' ', 3, "CLR %e" },
    
    { 0x80, 0xff, ' ', 2, "SUB A,%b" },
    { 0x81, 0xff, ' ', 2, "CMP A,%b" },
    { 0x82, 0xff, ' ', 2, "SBC A,%b" },
    { 0x84, 0xff, ' ', 2, "AND A,%b" },
    { 0x85, 0xff, ' ', 2, "BIT A,%b" },
    { 0x86, 0xff, ' ', 2, "LDA A,%b" },
    { 0x88, 0xff, ' ', 2, "EOR A,%b" },
    { 0x89, 0xff, ' ', 2, "ADC A,%b" },
    { 0x8a, 0xff, ' ', 2, "ORA A,%b" },
    { 0x8b, 0xff, ' ', 2, "ADD A,%b" },
    { 0x8c, 0xff, ' ', 3, "CPX %B" },
    { 0x8d, 0xff, ' ', 2, "BSR %r" },
    { 0x8e, 0xff, ' ', 3, "LDS %B" },

    { 0x90, 0xff, ' ', 2, "SUB A,%d" },
    { 0x91, 0xff, ' ', 2, "CMP A,%d" },
    { 0x92, 0xff, ' ', 2, "SBC A,%d" },
    { 0x94, 0xff, ' ', 2, "AND A,%d" },
    { 0x95, 0xff, ' ', 2, "BIT A,%d" },
    { 0x96, 0xff, ' ', 2, "LDA A,%d" },
    { 0x97, 0xff, ' ', 2, "STA A,%d" },
    { 0x98, 0xff, ' ', 2, "EOR A,%d" },
    { 0x99, 0xff, ' ', 2, "ADC A,%d" },
    { 0x9a, 0xff, ' ', 2, "ORA A,%d" },
    { 0x9b, 0xff, ' ', 2, "ADD A,%d" },
    { 0x9c, 0xff, ' ', 2, "CPX %D" },
    { 0x9e, 0xff, ' ', 2, "LDS %d" },
    { 0x9f, 0xff, ' ', 2, "STS %d" },

    { 0xa0, 0xff, ' ', 2, "SUB A,%x" },
    { 0xa1, 0xff, ' ', 2, "CMP A,%x" },
    { 0xa2, 0xff, ' ', 2, "SBC A,%x" },
    { 0xa4, 0xff, ' ', 2, "AND A,%x" },
    { 0xa5, 0xff, ' ', 2, "BIT A,%x" },
    { 0xa6, 0xff, ' ', 2, "LDA A,%x" },
    { 0xa7, 0xff, ' ', 2, "STA A,%x" },
    { 0xa8, 0xff, ' ', 2, "EOR A,%x" },
    { 0xa9, 0xff, ' ', 2, "ADC A,%x" },
    { 0xaa, 0xff, ' ', 2, "ORA A,%x" },
    { 0xab, 0xff, ' ', 2, "ADD A,%x" },
    { 0xac, 0xff, ' ', 2, "CPX %X" },
    { 0xad, 0xff, '_', 2, "JSR %X" },
    { 0xae, 0xff, ' ', 2, "LDS %X" },
    { 0xaf, 0xff, ' ', 2, "STS %X" },

    { 0xb0, 0xff, ' ', 3, "SUB A,%e" },
    { 0xb1, 0xff, ' ', 3, "CMP A,%e" },
    { 0xb2, 0xff, ' ', 3, "SBC A,%e" },
    { 0xb4, 0xff, ' ', 3, "AND A,%e" },
    { 0xb5, 0xff, ' ', 3, "BIT A,%e" },
    { 0xb6, 0xff, ' ', 3, "LDA A,%e" },
    { 0xb7, 0xff, ' ', 3, "STA A,%e" },
    { 0xb8, 0xff, ' ', 3, "EOR A,%e" },
    { 0xb9, 0xff, ' ', 3, "ADC A,%e" },
    { 0xba, 0xff, ' ', 3, "ORA A,%e" },
    { 0xbb, 0xff, ' ', 3, "ADD A,%e" },
    { 0xbc, 0xff, ' ', 3, "CPX %E" },
    { 0xbd, 0xff, 'E', 3, "JSR %e" },
    { 0xbe, 0xff, ' ', 3, "LDS %E" },
    { 0xbf, 0xff, ' ', 3, "STS %E" },

    { 0xc0, 0xff, ' ', 2, "SUB B,%b" },
    { 0xc1, 0xff, ' ', 2, "CMP B,%b" },
    { 0xc2, 0xff, ' ', 2, "SBC B,%b" },
    { 0xc4, 0xff, ' ', 2, "AND B,%b" },
    { 0xc5, 0xff, ' ', 2, "BIT B,%b" },
    { 0xc6, 0xff, ' ', 2, "LDA B,%b" },
    { 0xc8, 0xff, ' ', 2, "EOR B,%b" },
    { 0xc9, 0xff, ' ', 2, "ADC B,%b" },
    { 0xca, 0xff, ' ', 2, "ORA B,%b" },
    { 0xcb, 0xff, ' ', 2, "ADD B,%b" },
    { 0xce, 0xff, ' ', 3, "LDX %B" },

    { 0xd0, 0xff, ' ', 2, "SUB B,%d" },
    { 0xd1, 0xff, ' ', 2, "CMP B,%d" },
    { 0xd2, 0xff, ' ', 2, "SBC B,%d" },
    { 0xd4, 0xff, ' ', 2, "AND B,%d" },
    { 0xd5, 0xff, ' ', 2, "BIT B,%d" },
    { 0xd6, 0xff, ' ', 2, "LDA B,%d" },
    { 0xd7, 0xff, ' ', 2, "STA B,%d" },
    { 0xd8, 0xff, ' ', 2, "EOR B,%d" },
    { 0xd9, 0xff, ' ', 2, "ADC B,%d" },
    { 0xda, 0xff, ' ', 2, "ORA B,%d" },
    { 0xdb, 0xff, ' ', 2, "ADD B,%d" },
    { 0xde, 0xff, ' ', 2, "LDX %d" },
    { 0xdf, 0xff, ' ', 2, "STX %d" },

    { 0xe0, 0xff, ' ', 2, "SUB B,%x" },
    { 0xe1, 0xff, ' ', 2, "CMP B,%x" },
    { 0xe2, 0xff, ' ', 2, "SBC B,%x" },
    { 0xe4, 0xff, ' ', 2, "AND B,%x" },
    { 0xe5, 0xff, ' ', 2, "BIT B,%x" },
    { 0xe6, 0xff, ' ', 2, "LDA B,%x" },
    { 0xe7, 0xff, ' ', 2, "STA B,%x" },
    { 0xe8, 0xff, ' ', 2, "EOR B,%x" },
    { 0xe9, 0xff, ' ', 2, "ADC B,%x" },
    { 0xea, 0xff, ' ', 2, "ORA B,%x" },
    { 0xeb, 0xff, ' ', 2, "ADD B,%x" },
    { 0xee, 0xff, ' ', 2, "LDX %x" },
    { 0xef, 0xff, ' ', 2, "STX %x" },

    { 0xf0, 0xff, ' ', 3, "SUB B,%e" },
    { 0xf1, 0xff, ' ', 3, "CMP B,%e" },
    { 0xf2, 0xff, ' ', 3, "SBC B,%e" },
    { 0xf4, 0xff, ' ', 3, "AND B,%e" },
    { 0xf5, 0xff, ' ', 3, "BIT B,%e" },
    { 0xf6, 0xff, ' ', 3, "LDA B,%e" },
    { 0xf7, 0xff, ' ', 3, "STA B,%e" },
    { 0xf8, 0xff, ' ', 3, "EOR B,%e" },
    { 0xf9, 0xff, ' ', 3, "ADC B,%e" },
    { 0xfa, 0xff, ' ', 3, "ORA B,%e" },
    { 0xfb, 0xff, ' ', 3, "ADD B,%e" },
    { 0xfe, 0xff, ' ', 3, "LDX %e" },
    { 0xff, 0xff, ' ', 3, "STX %e" },

    { 0, 0, 0, 0, 0, 0 }
  };

/* End of motorola.src/glob68.cc */
