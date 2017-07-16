//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2014-2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//-------------------------------------------------------------------------------


//
// Title: Memory Pagination Utilities
//
// Constants to paginate memory
//

#ifndef CPCT_BANKING_H
#define CPCT_BANKING_H

// Info from http://www.cpcwiki.eu/index.php/Gate_Array#Register_3_-_RAM_Banking
// Memory banking is done in the CPC by using the Register 3 in the Gate Array.
// The memory configuration is defined by specifiying which bank of 64 kb to use as, 
// additional memory, and a memory layout (how the low 64 kb and the selected additional 
// 64kb are mapped to the low 64kb address range).
// Those two parameters are specified as groups of three bits in register 3 of the
// Gate Array.
// 
// Bit	Value	Function
// 7	1	Gate Array function 3
// 6	1
// 5	b	64K bank number (0..7); always 0 on an unexpanded CPC6128, 0-7 on Standard Memory Expansions
// 4	b
// 3	b
// 2	x	RAM Config (0..7)
// 1	x
// 0	x

#define BANK_NUM_SHIFT 3

// BANK NUMBERS:
// As these constants are specified in bits 3-5, the constants are 
// shifted so that they can be OR'd with the RAM configuration parameter.

// BANK_0: RAM_4 -> 10000-13FFF, RAM_5 -> 14000-17FFF, RAM_6 -> 18000-1BFFF, RAM_7 -> 1C000-1FFFF
#define BANK_0 (0 << BANK_NUM_SHIFT)
// BANK_1: RAM_4 -> 20000-23FFF, RAM_5 -> 24000-27FFF, RAM_6 -> 28000-1BFFF, RAM_7 -> 2C000-1FFFF
#define BANK_1 (1 << BANK_NUM_SHIFT)
// BANK_2: RAM_4 -> 30000-33FFF, RAM_5 -> 34000-37FFF, RAM_6 -> 38000-3BFFF, RAM_7 -> 3C000-3FFFF
#define BANK_2 (2 << BANK_NUM_SHIFT)
// BANK_3: RAM_4 -> 40000-43FFF, RAM_5 -> 44000-47FFF, RAM_6 -> 48000-4BFFF, RAM_7 -> 4C000-4FFFF
#define BANK_3 (3 << BANK_NUM_SHIFT)
// BANK_4: RAM_4 -> 50000-53FFF, RAM_5 -> 54000-57FFF, RAM_6 -> 58000-5BFFF, RAM_7 -> 5C000-5FFFF
#define BANK_4 (4 << BANK_NUM_SHIFT)
// BANK_5: RAM_4 -> 60000-63FFF, RAM_5 -> 64000-67FFF, RAM_6 -> 68000-6BFFF, RAM_7 -> 6C000-6FFFF
#define BANK_5 (5 << BANK_NUM_SHIFT)
// BANK_6: RAM_4 -> 70000-73FFF, RAM_5 -> 74000-77FFF, RAM_6 -> 78000-7BFFF, RAM_7 -> 7C000-7FFFF
#define BANK_6 (6 << BANK_NUM_SHIFT)
// BANK_7: RAM_4 -> 80000-83FFF, RAM_5 -> 84000-87FFF, RAM_6 -> 88000-8BFFF, RAM_7 -> 8C000-8FFFF
#define BANK_7 (7 << BANK_NUM_SHIFT)

// RAM CONFIGURATIONS:
// Specify which 16kb pages are mapped to each page in the addresable RAM range.

// RAMCFG_0: 0000-3FFF -> RAM_0, 4000-7FFF -> RAM_1, 8000-BFFF -> RAM_2, C000-FFFF -> RAM_3
// Only the lower 64kb are accessible.
#define RAMCFG_0 0
// RAMCFG_1: 0000-3FFF -> RAM_0, 4000-7FFF -> RAM_1, 8000-BFFF -> RAM_2, C000-FFFF -> RAM_7
#define RAMCFG_1 1
// RAMCFG_2: 0000-3FFF -> RAM_4, 4000-7FFF -> RAM_5, 8000-BFFF -> RAM_6, C000-FFFF -> RAM_7
#define RAMCFG_2 2
// RAMCFG_3: 0000-3FFF -> RAM_0, 4000-7FFF -> RAM_3, 8000-BFFF -> RAM_2, C000-FFFF -> RAM_7
#define RAMCFG_3 3
// RAMCFG_4: 0000-3FFF -> RAM_0, 4000-7FFF -> RAM_4, 8000-BFFF -> RAM_2, C000-FFFF -> RAM_3
#define RAMCFG_4 4
// RAMCFG_5: 0000-3FFF -> RAM_0, 4000-7FFF -> RAM_5, 8000-BFFF -> RAM_2, C000-FFFF -> RAM_3
#define RAMCFG_5 5
// RAMCFG_6: 0000-3FFF -> RAM_0, 4000-7FFF -> RAM_6, 8000-BFFF -> RAM_2, C000-FFFF -> RAM_3
#define RAMCFG_6 6
// RAMCFG_7: 0000-3FFF -> RAM_0, 4000-7FFF -> RAM_7, 8000-BFFF -> RAM_2, C000-FFFF -> RAM_3
#define RAMCFG_7 7

// The default memory configuration uses the standard upper 64kb RAM
// but it is not addressable, as the RAM configuration has mapped only 
// the lower 64kb.
#define DEFAULT_MEM_CFG RAMCFG_0 | BANK_0

#endif
