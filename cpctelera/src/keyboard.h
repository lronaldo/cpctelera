//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2014 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//-------------------------------------------------------------------------------

#ifndef CPCT_KEYBOARD_H
#define CPCT_KEYBOARD_H

///
/// Function Declarations
///
extern void cpct_scanKeyboard();
extern void cpct_scanKeyboardFast();
extern unsigned char cpct_isKeyPressed(unsigned int key);

///
/// KEY DEFINITIONS
/// Enumerated value with symbols for all the 80 possible Key/Joy definitions
///
typedef enum cpct_keyID
{
  // Matrix Line 00h
  Key_CursorUp     = 0x0100,  // Bit 0 (01h) => | 0000 0001 |
  Key_CursorRight  = 0x0200,  // Bit 1 (02h) => | 0000 0010 |
  Key_CursorDown   = 0x0400,  // Bit 2 (04h) => | 0000 0100 |
  Key_F9           = 0x0800,  // Bit 3 (08h) => | 0000 1000 |
  Key_F6           = 0x1000,  // Bit 4 (10h) => | 0001 0000 |
  Key_F3           = 0x2000,  // Bit 5 (20h) => | 0010 0000 |
  Key_Enter        = 0x4000,  // Bit 6 (40h) => | 0100 0000 |
  Key_FDot         = 0x8000,  // Bit 7 (80h) => | 1000 0000 |

  // Matrix Line 01h
  Key_CursorLeft   = 0x0101,
  Key_Copy         = 0x0201,
  Key_F7           = 0x0401,
  Key_F8           = 0x0801,
  Key_F5           = 0x1001,
  Key_F1           = 0x2001,
  Key_F2           = 0x4001,
  Key_F0           = 0x8001,

  // Matrix Line 02h 
  Key_Clr          = 0x0102,
  Key_BraceOpen    = 0x0202,
  Key_Return       = 0x0402,
  Key_BraceClose   = 0x0802,
  Key_F4           = 0x1002,
  Key_Shift        = 0x2002,
  Key_BackSlash    = 0x4002,
  Key_Control      = 0x8002,
  
  // Matrix Line 03h   
  Key_Caret        = 0x0103,
  Key_Hyphen       = 0x0203,
  Key_At           = 0x0403,
  Key_P            = 0x0803,
  Key_SemiColon    = 0x1003,
  Key_Colon        = 0x2003,
  Key_Slash        = 0x4003,
  Key_Dot          = 0x8003,

  // Matrix Line 04h
  Key_0            = 0x0104,
  Key_9            = 0x0204,
  Key_O            = 0x0404,
  Key_I            = 0x0804,
  Key_L            = 0x1004,
  Key_K            = 0x2004,
  Key_M            = 0x4004,
  Key_Comma        = 0x8004,

  // Matrix Line 05h
  Key_8            = 0x0105,
  Key_7            = 0x0205,
  Key_U            = 0x0405,
  Key_Y            = 0x0805,
  Key_H            = 0x1005,
  Key_J            = 0x2005,
  Key_N            = 0x4005,
  Key_Space        = 0x8005,

  // Matrix Line 06h
  Key_6            = 0x0106,
  Key_Joy1Up       = 0x0106,
  Key_5            = 0x0206,
  Key_Joy1Down     = 0x0206,
  Key_R            = 0x0406,
  Key_Joy1Left     = 0x0406,
  Key_T            = 0x0806,
  Key_Joy1Right    = 0x0806,
  Key_G            = 0x1006,
  Key_Joy1Fire2    = 0x1006,
  Key_F            = 0x2006,
  Key_Joy1Fire1    = 0x2006,
  Key_B            = 0x4006,
  Key_Joy1Fire3    = 0x4006,
  Key_V            = 0x8006,

  // Matrix Line 07h
  Key_4            = 0x0107,
  Key_3            = 0x0207,
  Key_E            = 0x0407,
  Key_W            = 0x0807,
  Key_S            = 0x1007,
  Key_D            = 0x2007,
  Key_C            = 0x4007,
  Key_X            = 0x8007,

  // Matrix Line 08h
  Key_1            = 0x0108,
  Key_2            = 0x0208,
  Key_Esc          = 0x0408,
  Key_Q            = 0x0808,
  Key_Tab          = 0x1008,
  Key_A            = 0x2008,
  Key_CapsLock     = 0x4008,
  Key_Z            = 0x8008,
  
  // Matrix Line 09h
  Key_Joy0Up       = 0x0109,
  Key_Joy0Down     = 0x0209,
  Key_Joy0Left     = 0x0409,
  Key_Joy0Right    = 0x0809,
  Key_Joy0Fire1    = 0x1009,
  Key_Joy0Fire2    = 0x2009,
  Key_Joy0Fire3    = 0x4009,
  Key_Del          = 0x8009
} cpct_keyID;
#endif
