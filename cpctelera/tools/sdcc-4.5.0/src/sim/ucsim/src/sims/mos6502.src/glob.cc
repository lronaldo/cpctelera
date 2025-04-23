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

#include "glob.h"


instruction_wrapper_fn itab[256];

/*	Formats
	x (ind,X)
	y (ind),Y
	a abs
	z zpg
	X zpg,X
	Y zpg,Y
	i abs,X
	p abs.Y
	r rel
	# imm8
	3 (ind)
	4 (zind)
*/

// code mask branch len mn call tick
struct dis_entry disass_mos6502[]=
  {
    { 0xea, 0xff, ' ', 1, "NOP" },
    { 0x00, 0xff, ' ', 1, "BRK" },
    { 0x40, 0xff, 'x', 1, "RTI" },
    { 0x58, 0xff, ' ', 1, "CLI" },
    { 0x78, 0xff, ' ', 1, "SEI" },
    { 0x08, 0xff, ' ', 1, "PHP" },
    { 0x18, 0xff, ' ', 1, "CLC" },
    { 0x28, 0xff, ' ', 1, "PLP" },
    { 0x38, 0xff, ' ', 1, "SEC" },
    { 0x48, 0xff, ' ', 1, "PHA" },
    { 0x68, 0xff, ' ', 1, "PLA" },
    { 0x88, 0xff, ' ', 1, "DEY" },
    { 0x98, 0xff, ' ', 1, "TYA" },
    { 0xa8, 0xff, ' ', 1, "TAY" },
    { 0xb8, 0xff, ' ', 1, "CLV" },
    { 0xc8, 0xff, ' ', 1, "INY" },
    { 0xd8, 0xff, ' ', 1, "CLD" },
    { 0xe8, 0xff, ' ', 1, "INX" },
    { 0xf8, 0xff, ' ', 1, "SED" },
    { 0x8a, 0xff, ' ', 1, "TXA" },
    { 0x9a, 0xff, ' ', 1, "TXS" },
    { 0xaa, 0xff, ' ', 1, "TAX" },
    { 0xba, 0xff, ' ', 1, "TSX" },
    { 0xca, 0xff, ' ', 1, "DEX" },

    { 0x01, 0xff, ' ', 2, "ORA %x" },
    { 0x11, 0xff, ' ', 2, "ORA %y" },
    { 0x05, 0xff, ' ', 2, "ORA %z" },
    { 0x15, 0xff, ' ', 2, "ORA %X" },
    { 0x09, 0xff, ' ', 2, "ORA %#" },
    { 0x19, 0xff, ' ', 3, "ORA %p" },
    { 0x0d, 0xff, ' ', 3, "ORA %a" },
    { 0x1d, 0xff, ' ', 3, "ORA %i" },

    { 0x21, 0xff, ' ', 2, "AND %x" },
    { 0x31, 0xff, ' ', 2, "AND %y" },
    { 0x25, 0xff, ' ', 2, "AND %z" },
    { 0x35, 0xff, ' ', 2, "AND %X" },
    { 0x29, 0xff, ' ', 2, "AND %#" },
    { 0x39, 0xff, ' ', 3, "AND %p" },
    { 0x2d, 0xff, ' ', 3, "AND %a" },
    { 0x3d, 0xff, ' ', 3, "AND %i" },

    { 0x41, 0xff, ' ', 2, "EOR %x" },
    { 0x51, 0xff, ' ', 2, "EOR %y" },
    { 0x45, 0xff, ' ', 2, "EOR %z" },
    { 0x55, 0xff, ' ', 2, "EOR %X" },
    { 0x49, 0xff, ' ', 2, "EOR %#" },
    { 0x59, 0xff, ' ', 3, "EOR %p" },
    { 0x4d, 0xff, ' ', 3, "EOR %a" },
    { 0x5d, 0xff, ' ', 3, "EOR %i" },

    { 0x61, 0xff, ' ', 2, "ADC %x" },
    { 0x71, 0xff, ' ', 2, "ADC %y" },
    { 0x65, 0xff, ' ', 2, "ADC %z" },
    { 0x75, 0xff, ' ', 2, "ADC %X" },
    { 0x69, 0xff, ' ', 2, "ADC %#" },
    { 0x79, 0xff, ' ', 3, "ADC %p" },
    { 0x6d, 0xff, ' ', 3, "ADC %a" },
    { 0x7d, 0xff, ' ', 3, "ADC %i" },

    { 0x81, 0xff, ' ', 2, "STA %x" },
    { 0x91, 0xff, ' ', 2, "STA %y" },
    { 0x85, 0xff, ' ', 2, "STA %z" },
    { 0x95, 0xff, ' ', 2, "STA %X" },
    { 0x99, 0xff, ' ', 3, "STA %p" },
    { 0x8d, 0xff, ' ', 3, "STA %a" },
    { 0x9d, 0xff, ' ', 3, "STA %i" },

    { 0xa1, 0xff, ' ', 2, "LDA %x" },
    { 0xb1, 0xff, ' ', 2, "LDA %y" },
    { 0xa5, 0xff, ' ', 2, "LDA %z" },
    { 0xb5, 0xff, ' ', 2, "LDA %X" },
    { 0xa9, 0xff, ' ', 2, "LDA %#" },
    { 0xb9, 0xff, ' ', 3, "LDA %p" },
    { 0xad, 0xff, ' ', 3, "LDA %a" },
    { 0xbd, 0xff, ' ', 3, "LDA %i" },

    { 0xc1, 0xff, ' ', 2, "CMP %x" },
    { 0xd1, 0xff, ' ', 2, "CMP %y" },
    { 0xc5, 0xff, ' ', 2, "CMP %z" },
    { 0xd5, 0xff, ' ', 2, "CMP %X" },
    { 0xc9, 0xff, ' ', 2, "CMP %#" },
    { 0xd9, 0xff, ' ', 3, "CMP %p" },
    { 0xcd, 0xff, ' ', 3, "CMP %a" },
    { 0xdd, 0xff, ' ', 3, "CMP %i" },

    { 0xe1, 0xff, ' ', 2, "SBC %x" },
    { 0xf1, 0xff, ' ', 2, "SBC %y" },
    { 0xe5, 0xff, ' ', 2, "SBC %z" },
    { 0xf5, 0xff, ' ', 2, "SBC %X" },
    { 0xe9, 0xff, ' ', 2, "SBC %#" },
    { 0xf9, 0xff, ' ', 3, "SBC %p" },
    { 0xed, 0xff, ' ', 3, "SBC %a" },
    { 0xfd, 0xff, ' ', 3, "SBC %i" },

    { 0x84, 0xff, ' ', 2, "STY %z" },
    { 0x94, 0xff, ' ', 2, "STY %X" },
    { 0x8c, 0xff, ' ', 3, "STY %a" },
    { 0x86, 0xff, ' ', 2, "STX %z" },
    { 0x96, 0xff, ' ', 2, "STX %Y" },
    { 0x8e, 0xff, ' ', 3, "STX %a" },

    { 0xa0, 0xff, ' ', 2, "LDY %#" },
    { 0xa4, 0xff, ' ', 2, "LDY %z" },
    { 0xb4, 0xff, ' ', 2, "LDY %X" },
    { 0xac, 0xff, ' ', 3, "LDY %a" },
    { 0xbc, 0xff, ' ', 3, "LDY %i" },
    { 0xa2, 0xff, ' ', 2, "LDX %#" },
    { 0xa6, 0xff, ' ', 2, "LDX %z" },
    { 0xb6, 0xff, ' ', 2, "LDX %Y" },
    { 0xae, 0xff, ' ', 3, "LDX %a" },
    { 0xbe, 0xff, ' ', 3, "LDX %p" },

    { 0xc0, 0xff, ' ', 2, "CPY %#" },
    { 0xc4, 0xff, ' ', 2, "CPY %z" },
    { 0xcc, 0xff, ' ', 3, "CPY %a" },
    { 0xe0, 0xff, ' ', 2, "CPX %#" },
    { 0xe4, 0xff, ' ', 2, "CPX %z" },
    { 0xec, 0xff, ' ', 3, "CPX %a" },

    { 0xe6, 0xff, ' ', 2, "INC %z" },
    { 0xf6, 0xff, ' ', 2, "INC %X" },
    { 0xee, 0xff, ' ', 3, "INC %a" },
    { 0xfe, 0xff, ' ', 3, "INC %i" },
    { 0xc6, 0xff, ' ', 2, "DEC %z" },
    { 0xd6, 0xff, ' ', 2, "DEC %X" },
    { 0xce, 0xff, ' ', 3, "DEC %a" },
    { 0xde, 0xff, ' ', 3, "DEC %i" },

    { 0x06, 0xff, ' ', 2, "ASL %z" },
    { 0x16, 0xff, ' ', 2, "ASL %X" },
    { 0x0a, 0xff, ' ', 1, "ASL A" },
    { 0x0e, 0xff, ' ', 3, "ASL %a" },
    { 0x1e, 0xff, ' ', 3, "ASL %i" },

    { 0x46, 0xff, ' ', 2, "LSR %z" },
    { 0x56, 0xff, ' ', 2, "LSR %X" },
    { 0x4a, 0xff, ' ', 1, "LSR A" },
    { 0x4e, 0xff, ' ', 3, "LSR %a" },
    { 0x5e, 0xff, ' ', 3, "LSR %i" },

    { 0x26, 0xff, ' ', 2, "ROL %z" },
    { 0x36, 0xff, ' ', 2, "ROL %X" },
    { 0x2a, 0xff, ' ', 1, "ROL A" },
    { 0x2e, 0xff, ' ', 3, "ROL %a" },
    { 0x3e, 0xff, ' ', 3, "ROL %i" },

    { 0x66, 0xff, ' ', 2, "ROR %z" },
    { 0x76, 0xff, ' ', 2, "ROR %X" },
    { 0x6a, 0xff, ' ', 1, "ROR A" },
    { 0x6e, 0xff, ' ', 3, "ROR %a" },
    { 0x7e, 0xff, ' ', 3, "ROR %i" },

    { 0x24, 0xff, ' ', 2, "BIT %z" },
    { 0x2c, 0xff, ' ', 3, "BIT %a" },

    { 0x4c, 0xff, 'j', 3, "JMP %j" },
    { 0x6c, 0xff, 'x', 3, "JMP %J" },
    
    { 0x20, 0xff, 's', 3, "JSR %j" },
    { 0x60, 0xff, 'x', 1, "RTS" },

    { 0x10, 0xff, 'b', 2, "BPL %r" },
    { 0x30, 0xff, 'b', 2, "BMI %r" },
    { 0x50, 0xff, 'b', 2, "BVC %r" },
    { 0x70, 0xff, 'b', 2, "BVS %r" },
    { 0x90, 0xff, 'b', 2, "BCC %r" },
    { 0xb0, 0xff, 'b', 2, "BCS %r" },
    { 0xd0, 0xff, 'b', 2, "BNE %r" },
    { 0xf0, 0xff, 'b', 2, "BEQ %r" },
    
    { 0, 0, 0, 0, 0, 0 }
  };

struct dis_entry disass_mos65c02[]=
  {
    { 0x80, 0xff, 'b', 2, "BRA %r" },
    
    { 0x02, 0xff, ' ', 2, "NOP %#" },
    { 0x12, 0xff, ' ', 2, "ORA %4" },
    { 0x22, 0xff, ' ', 2, "NOP %#" },
    { 0x32, 0xff, ' ', 2, "AND %4" },
    { 0x42, 0xff, ' ', 2, "NOP %#" },
    { 0x52, 0xff, ' ', 2, "EOR %4" },
    { 0x62, 0xff, ' ', 2, "NOP %#" },
    { 0x72, 0xff, ' ', 2, "ADC %4" },
    { 0x82, 0xff, ' ', 2, "NOP %#" },
    { 0x92, 0xff, ' ', 2, "STA %4" },
    { 0xb2, 0xff, ' ', 2, "LDA %4" },
    { 0xc2, 0xff, ' ', 2, "NOP %#" },
    { 0xd2, 0xff, ' ', 2, "CMP %4" },
    { 0xe2, 0xff, ' ', 2, "NOP %#" },
    { 0xf2, 0xff, ' ', 2, "SBC %4" },

    { 0x04, 0xff, ' ', 2, "TSB %z" },
    { 0x14, 0xff, ' ', 2, "TRB %z" },
    { 0x34, 0xff, ' ', 2, "BIT %X" },
    { 0x44, 0xff, ' ', 2, "NOP %z" },
    { 0x54, 0xff, ' ', 2, "NOP %X" },
    { 0x64, 0xff, ' ', 2, "STZ %z" },
    { 0x74, 0xff, ' ', 2, "STZ %X" },
    { 0xd4, 0xff, ' ', 2, "NOP %X" },
    { 0xf4, 0xff, ' ', 2, "NOP %X" },

    { 0x89, 0xff, ' ', 2, "BIT %#" },

    { 0x1a, 0xff, ' ', 1, "INA" },
    { 0x3a, 0xff, ' ', 1, "DEA" },
    { 0x5a, 0xff, ' ', 1, "PHY" },
    { 0x7a, 0xff, ' ', 1, "PLY" },
    { 0xda, 0xff, ' ', 1, "PHX" },
    { 0xfa, 0xff, ' ', 1, "PLX" },
    
    { 0x0c, 0xff, ' ', 3, "TSB %a" },
    { 0x1c, 0xff, ' ', 3, "TRB %a" },
    { 0x3c, 0xff, ' ', 3, "BIT %i" },
    { 0x5c, 0xff, ' ', 3, "NOP %a" },
    { 0x7c, 0xff, 'x', 3, "JMP %I" },
    { 0xdc, 0xff, ' ', 3, "NOP %a" },
    { 0xfc, 0xff, ' ', 3, "NOP %a" },
    { 0x9c, 0xff, ' ', 3, "STZ %a" },

    { 0x9e, 0xff, ' ', 3, "STZ %i" },

    { 0x03, 0x0f, ' ', 1, "NOP" },
    { 0x0b, 0x0f, ' ', 1, "NOP" },
    
    { 0x07, 0x8f, ' ', 1, "NOP" },
    { 0x87, 0x8f, ' ', 1, "NOP" },
    { 0x0f, 0x8f, ' ', 1, "NOP" },
    { 0x8f, 0x8f, ' ', 1, "NOP" },

    { 0, 0, 0, 0, 0, 0 }
  };
  
struct dis_entry disass_mos65c02s[]=
  {
    { 0xcb, 0xff, ' ', 1, "WAI" },
    { 0xdb, 0xff, ' ', 1, "STP" },
    
    { 0x07, 0x8f, ' ', 2, "RMB%B %z" },
    { 0x87, 0x8f, ' ', 2, "SMB%B %z" },
    { 0x0f, 0x8f, 'B', 3, "BBR%B %z,%R" },
    { 0x8f, 0x8f, 'B', 3, "BBS%B %z,%R" },
    
    { 0, 0, 0, 0, 0, 0 }
  };

struct cpu_entry cpus_6502[]=
  {
    {"6502"	, CPU_6502, 0		, "MOS6502", ""},
    {"02"	, CPU_6502, 0		, "MOS6502", ""},
    {"7501"	, CPU_7501, 0		, "MOS7501", ""},
    {"8501"	, CPU_8501, 0		, "MOS8501", ""},
    {"65C02"	, CPU_65C02, 0		, "MOS65C02", ""},
    {"C02"	, CPU_65C02, 0		, "MOS65C02", ""},
    {"C"	, CPU_65C02, 0		, "MOS65C02", ""},
    {"6510"	, CPU_6510, 0		, "MOS6510", ""},
    {"10"	, CPU_6510, 0		, "MOS6510", ""},
    {"8500"	, CPU_8500, 0		, "MOS8500", ""},
    {"8502"	, CPU_8502, 0		, "MOS8502", ""},
    {"65CE02"	, CPU_65CE02, 0		, "MOS65CE02", ""},
    {"CE02"	, CPU_65CE02, 0		, "MOS65CE02", ""},
    {"CE"	, CPU_65CE02, 0		, "MOS65CE02", ""},
    {"65C02S"	, CPU_65C02S, 0		, "MOS65C02S", ""},
    {"C02S"	, CPU_65C02S, 0		, "MOS65C02S", ""},
    {"CS"	, CPU_65C02S, 0		, "MOS65C02S", ""},
    {"S"	, CPU_65C02S, 0		, "MOS65C02S", ""},
    
    {NULL, CPU_NONE, 0, "", ""}
  };

/* End of mos6502.src/glob.cc */
