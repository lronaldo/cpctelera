//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
 //  Copyright (C) 2014 - 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

//
//#####################################################################
//### MODULE Keyboard                                               ###
//#####################################################################
//### Routines control and check keyboard status, keys and joystick ###
//#####################################################################
//

//
// File: keyboard.h
//

#ifndef CPCT_KEYBOARD_H
#define CPCT_KEYBOARD_H

#include <types.h>

//
// Declare type for CPC keys
//
        enum cpct_e_keyID;
typedef enum cpct_e_keyID cpct_keyID;

///
/// Function Declarations
///
extern void cpct_scanKeyboard     ();
extern void cpct_scanKeyboardFast ();
extern   u8 cpct_isKeyPressed     (cpct_keyID key);

//
// Enum: cpct_keyID
// 
//    Enumerated type with symbols for all the 80 possible Key/Joy definitions.
//
// Details:
//    Figure 1 shows the layout for an Amstrad CPC Keyboard, along with
// its firmware Key Codes. Firmware Key Codes (FKCs) are used in table 1 to map them
// to cpct_keyID enum values,
//
// (start code)
//                                                      __
//                                                     | 0|
//                                          ENC*     —— —— ——
//   AMSTRAD                   CPC464 RGB color     | 8| 9| 1|
//                                                   —— —— ——
//  __ __ __ __ __ __ __ __ __ __ __ __ __ __ ___      | 2|
// |66|64|65|57|56|49|48|41|40|33|32|25|24|16|79 |      ——
//  —— —— —— —— —— —— —— —— —— —— —— —— —— —— ———    —— —— ——
// |68 |67|59|58|50|51|43|42|35|34|27|26|17|     |  |10|11| 3|
//  ——— —— —— —— —— —— —— —— —— —— —— —— —— = 18 |   —— —— ——
// | 70 |69|60|61|53|52|44|45|37|36|29|28|19|    |  |20|12| 4|
//  ———— —— —— —— —— —— —— —— —— —— —— —— —— ————    —— —— ——
// | 21  |71|63|62|55|54|46|38|39|31|30|22|  21  |  |13|14| 5|
//  ————— —— —— —— —— —— —— —— —— —— —— —— ——————    —— —— ——
//          |            47            |23|         |15| 7| 6|
//           —————————————————————————— ——           —— —— ——
//      JOY 0   ___               JOY 1   ___        
//             | 72|                     | 48|       
//         ——|———————|——             ——|———————|——   
//        |74| 76| 77|75|           |50| 52| 53|51|  
//         ——|———————|——             ——|———————|——   
//             | 73|                     | 49|       
//              ———                       ———        
// ====================================================================
//  Figure 1. Amstrad CPC Keyoard Layout with Firmware Key Codes (FKCs)
// (end)
//
// (start code)
//  FKC | cpct_keyID      || FKC  | cpct_keyID    ||  FKC  |  cpct_keyID   
// --------------------------------------------------------------------
//    0 | Key_CursorUp    ||  27  | Key_P         ||   54  |  Key_B
//      |                 ||      |               ||       |  Key_Joy1Fire3
//    1 | Key_CursorRight ||  28  | Key_SemiColon ||   55  |  Key_V
//    2 | Key_CursorDown  ||  29  | Key_Colon     ||   56  |  Key_4
//    3 | Key_F9          ||  30  | Key_Slash     ||   57  |  Key_3
//    4 | Key_F6          ||  31  | Key_Dot       ||   58  |  Key_E
//    5 | Key_F3          ||  32  | Key_0         ||   59  |  Key_W
//    6 | Key_Enter       ||  33  | Key_9         ||   60  |  Key_S
//    7 | Key_FDot        ||  34  | Key_O         ||   61  |  Key_D
//    8 | Key_CursorLeft  ||  35  | Key_I         ||   62  |  Key_C
//    9 | Key_Copy        ||  36  | Key_L         ||   63  |  Key_X
//   10 | Key_F7          ||  37  | Key_K         ||   64  |  Key_1
//   11 | Key_F8          ||  38  | Key_M         ||   65  |  Key_2
//   12 | Key_F5          ||  39  | Key_Comma     ||   66  |  Key_Esc
//   13 | Key_F1          ||  40  | Key_8         ||   67  |  Key_Q
//   14 | Key_F2          ||  41  | Key_7         ||   68  |  Key_Tab
//   15 | Key_F0          ||  42  | Key_U         ||   69  |  Key_A
//   16 | Key_Clr         ||  43  | Key_Y         ||   70  |  Key_CapsLock
//   17 | Key_OpenBracket ||  44  | Key_H         ||   71  |  Key_Z
//   18 | Key_Return      ||  45  | Key_J         ||   72  |  Key_Joy0Up
//   19 | Key_CloseBracket||  46  | Key_N         ||   73  |  Key_Joy0Down
//   20 | Key_F4          ||  47  | Key_Space     ||   74  |  Key_Joy0Left
//   21 | Key_Shift       ||  48  | Key_6         ||   75  |  Key_Joy0Right
//      |                 ||      | Key_Joy1Up    ||
//   22 | Key_BackSlash   ||  49  | Key_5         ||   76  |  Key_Joy0Fire1
//      |                 ||      | Key_Joy1Down  ||
//   23 | Key_Control     ||  50  | Key_R         ||   77  |  Key_Joy0Fire2
//      |                 ||      | Key_Joy1Left  ||       |
//   24 | Key_Caret       ||  51  | Key_T         ||   78  |  Key_Joy0Fire3
//      |                 ||      | Key_Joy1Right ||
//   25 | Key_Hyphen      ||  52  | Key_G         ||   79  |  Key_Del
//      |                 ||      | Key_Joy1Fire1 ||
//   26 | Key_At          ||  53  | Key_F         ||
//      |                 ||      | Key_Joy1Fire2 ||
// --------------------------------------------------------------------
//  Table 1. cpct_keyIDs defined for each possible key, ordered by FKCs
// (end)
//
enum cpct_e_keyID
{
  // Matrix Line 00h
  Key_CursorUp     = (i16)0x0100,  // Bit 0 (01h) => | 0000 0001 |
  Key_CursorRight  = (i16)0x0200,  // Bit 1 (02h) => | 0000 0010 |
  Key_CursorDown   = (i16)0x0400,  // Bit 2 (04h) => | 0000 0100 |
  Key_F9           = (i16)0x0800,  // Bit 3 (08h) => | 0000 1000 |
  Key_F6           = (i16)0x1000,  // Bit 4 (10h) => | 0001 0000 |
  Key_F3           = (i16)0x2000,  // Bit 5 (20h) => | 0010 0000 |
  Key_Enter        = (i16)0x4000,  // Bit 6 (40h) => | 0100 0000 |
  Key_FDot         = (i16)0x8000,  // Bit 7 (80h) => | 1000 0000 |

  // Matrix Line 01h
  Key_CursorLeft   = (i16)0x0101,
  Key_Copy         = (i16)0x0201,
  Key_F7           = (i16)0x0401,
  Key_F8           = (i16)0x0801,
  Key_F5           = (i16)0x1001,
  Key_F1           = (i16)0x2001,
  Key_F2           = (i16)0x4001,
  Key_F0           = (i16)0x8001,

  // Matrix Line 02h
  Key_Clr          = (i16)0x0102,
  Key_OpenBracket  = (i16)0x0202,
  Key_Return       = (i16)0x0402,
  Key_CloseBracket = (i16)0x0802,
  Key_F4           = (i16)0x1002,
  Key_Shift        = (i16)0x2002,
  Key_BackSlash    = (i16)0x4002,
  Key_Control      = (i16)0x8002,

  // Matrix Line 03h
  Key_Caret        = (i16)0x0103,
  Key_Hyphen       = (i16)0x0203,
  Key_At           = (i16)0x0403,
  Key_P            = (i16)0x0803,
  Key_SemiColon    = (i16)0x1003,
  Key_Colon        = (i16)0x2003,
  Key_Slash        = (i16)0x4003,
  Key_Dot          = (i16)0x8003,

  // Matrix Line 04h
  Key_0            = (i16)0x0104,
  Key_9            = (i16)0x0204,
  Key_O            = (i16)0x0404,
  Key_I            = (i16)0x0804,
  Key_L            = (i16)0x1004,
  Key_K            = (i16)0x2004,
  Key_M            = (i16)0x4004,
  Key_Comma        = (i16)0x8004,

  // Matrix Line 05h
  Key_8            = (i16)0x0105,
  Key_7            = (i16)0x0205,
  Key_U            = (i16)0x0405,
  Key_Y            = (i16)0x0805,
  Key_H            = (i16)0x1005,
  Key_J            = (i16)0x2005,
  Key_N            = (i16)0x4005,
  Key_Space        = (i16)0x8005,

  // Matrix Line 06h
  Key_6            = (i16)0x0106,
  Key_Joy1Up       = (i16)0x0106,
  Key_5            = (i16)0x0206,
  Key_Joy1Down     = (i16)0x0206,
  Key_R            = (i16)0x0406,
  Key_Joy1Left     = (i16)0x0406,
  Key_T            = (i16)0x0806,
  Key_Joy1Right    = (i16)0x0806,
  Key_G            = (i16)0x1006,
  Key_Joy1Fire2    = (i16)0x1006,
  Key_F            = (i16)0x2006,
  Key_Joy1Fire1    = (i16)0x2006,
  Key_B            = (i16)0x4006,
  Key_Joy1Fire3    = (i16)0x4006,
  Key_V            = (i16)0x8006,

  // Matrix Line 07h
  Key_4            = (i16)0x0107,
  Key_3            = (i16)0x0207,
  Key_E            = (i16)0x0407,
  Key_W            = (i16)0x0807,
  Key_S            = (i16)0x1007,
  Key_D            = (i16)0x2007,
  Key_C            = (i16)0x4007,
  Key_X            = (i16)0x8007,

  // Matrix Line 08h
  Key_1            = (i16)0x0108,
  Key_2            = (i16)0x0208,
  Key_Esc          = (i16)0x0408,
  Key_Q            = (i16)0x0808,
  Key_Tab          = (i16)0x1008,
  Key_A            = (i16)0x2008,
  Key_CapsLock     = (i16)0x4008,
  Key_Z            = (i16)0x8008,

  // Matrix Line 09h
  Key_Joy0Up       = (i16)0x0109,
  Key_Joy0Down     = (i16)0x0209,
  Key_Joy0Left     = (i16)0x0409,
  Key_Joy0Right    = (i16)0x0809,
  Key_Joy0Fire1    = (i16)0x1009,
  Key_Joy0Fire2    = (i16)0x2009,
  Key_Joy0Fire3    = (i16)0x4009,
  Key_Del          = (i16)0x8009
};

#endif
