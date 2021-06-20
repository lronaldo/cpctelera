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
// Title: Memory Pagination Macros
//

#ifndef CPCT_BANKING_H
#define CPCT_BANKING_H

//
// Macro: BANK_NUM_SHIFT
//
//    Defines the number of bit to shift the number of a desired bank to the left
// if order to use it inside a 1-byte command for the Gate Array. Refer to the 
// tehcnical explanation to see bits labelled as "b" (bits 3-5) which specify
// the bank number to be selected. So, a normal number from 0 to 8 must be shifted
// left to be placed in the bits where it is expected by the Gate Array.
//
#define BANK_NUM_SHIFT 3

// Macros: BANK_X
//
//    Each one of this macros specify a memory bank configuration. This configuration
// is represented by a 3-bits number (0-8 in decimal) that must be passed to the Gate
// array to select the concrete memory configuration. Review documentation about
// <cpct_pageMemory> function for reference on RAM Banks.
//
// (start code)
//           |                  RAM Configuration
//    Macro  |     RAM_4          RAM_5          RAM_6          RAM_7
//  ---------|------------------------------------------------------------
//    BANK_0 |  10000-13FFF    14000-17FFF    18000-1BFFF    1C000-1FFFF
//    BANK_1 |  20000-23FFF    24000-27FFF    28000-1BFFF    2C000-1FFFF
//    BANK_2 |  30000-33FFF    34000-37FFF    38000-3BFFF    3C000-3FFFF
//    BANK_3 |  40000-43FFF    44000-47FFF    48000-4BFFF    4C000-4FFFF
//    BANK_4 |  50000-53FFF    54000-57FFF    58000-5BFFF    5C000-5FFFF
//    BANK_5 |  60000-63FFF    64000-67FFF    68000-6BFFF    6C000-6FFFF
//    BANK_6 |  70000-73FFF    74000-77FFF    78000-7BFFF    7C000-7FFFF
//    BANK_7 |  80000-83FFF    84000-87FFF    88000-8BFFF    8C000-8FFFF
// (end code)
//
#define BANK_0 (0 << BANK_NUM_SHIFT)
#define BANK_1 (1 << BANK_NUM_SHIFT)
#define BANK_2 (2 << BANK_NUM_SHIFT)
#define BANK_3 (3 << BANK_NUM_SHIFT)
#define BANK_4 (4 << BANK_NUM_SHIFT)
#define BANK_5 (5 << BANK_NUM_SHIFT)
#define BANK_6 (6 << BANK_NUM_SHIFT)
#define BANK_7 (7 << BANK_NUM_SHIFT)

// Macros: RAMCFG_X
//
//    Each one of this macros specify a RAM Configuration (how 16K memory pages
// are spread among the 64K available memory space). The Gate Array has 3 bits
// to specify the RAM configuration, so 8 posible configurations are available.
// These are the memory configurations:
//
// (start code)
//             |    PAGE 0         PAGE 1         PAGE 2         PAGE 3
//    Macro    |   0000-3FFF      4000-7FFF      8000-BFFF      C000-FFFF
//  -----------|------------------------------------------------------------
//    RAMCFG_0 |    RAM_0           RAM_1          RAM_2          RAM_3
//    RAMCFG_1 |    RAM_0           RAM_1          RAM_2          RAM_7
//    RAMCFG_2 |    RAM_4           RAM_5          RAM_6          RAM_7
//    RAMCFG_3 |    RAM_0           RAM_3          RAM_2          RAM_7
//    RAMCFG_4 |    RAM_0           RAM_4          RAM_2          RAM_3
//    RAMCFG_5 |    RAM_0           RAM_5          RAM_2          RAM_3
//    RAMCFG_6 |    RAM_0           RAM_6          RAM_2          RAM_3
//    RAMCFG_7 |    RAM_0           RAM_7          RAM_2          RAM_3
// (end code)
//
#define RAMCFG_0 0
#define RAMCFG_1 1
#define RAMCFG_2 2
#define RAMCFG_3 3
#define RAMCFG_4 4
#define RAMCFG_5 5
#define RAMCFG_6 6
#define RAMCFG_7 7

// Macro: DEFAULT_MEM_CFG
//
//    This represents the standard configuration of the system, where 
// the address space only maps the lower 64K (RAMCFG_0 | BANK_0). This
// is the configuration the system boots in. This default memory 
// configuration uses the standard upper 64kb RAM but they are not 
// addressable, as the RAM configuration has mapped only the lower 64kb.
//
#define DEFAULT_MEM_CFG RAMCFG_0 | BANK_0

#endif

// Section: Technical details on memory pagination
//
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
