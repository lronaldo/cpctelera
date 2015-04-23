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
//### MODULE: Keyboard                                              ###
//#####################################################################
//### Routines control and check keyboard status, keys and joystick ###
//#####################################################################
//### TECHNICAL INFORMATION:                                        ###
//### Keyboard and joystick are connected to AY-3-8912 Programable  ###
//### Sound Generator (PSG) which recieves, processes and stores    ###
//### pressed/not pressed information. The PSG is connected to the  ###
//### 8255 Programable Peripheral Interface (PPI), which the CPU    ###
//### can directly address. Therefore, to read the keyboard and     ###
//### joystick status, the Z80 has to communicate with the PPI and  ###
//### ask it to read the PSG status. This is done using OUT         ###
//### instruction to the 8255 PPI ports:                            ###
//###  > PPI Port A           = #F4xx                               ###
//###  > PPI Port B           = #F5xx                               ###
//###  > PPI Port C           = #F6xx                               ###
//###  > PPI Control-Register = #F7xx                               ###
//###                                                               ###
//### Keyboard and joystick switches and buttons are arranged in a  ###
//### 10x8 matrix. Each element of the matrix represents the state  ###
//### of one button/switch/key (pressed/not pressed). That means    ###
//### the CPC can control up to 80 keys/switches/buttons in total.  ###
//###                                                               ###
//### We're able to read a complete column of the matrix each time. ###
//### That means we get the state of 8 switches at a time, in the   ###
//### form of a byte (each bit represents the state of an switch).  ###
//### Each bit will hold a "0" if the switch is "pressed" or a "1"  ###
//### when the switch is "not pressed".                             ###
//###                                                               ###
//### It is relevant to notice something about joysticks. Although  ###
//### joystick 0 has its own column in the matrix (9th) to control  ###
//### its 6 switches, joystick 1 shares its switches with other     ###
//### keys in the 6th column (namely: F, G, T, R, 5, 6). Therefore, ###
//### it is possible to emulate a 2nd joystick using keyboard. The  ###
//### exact mapping between matrix values and switches pressed is   ###
//### included below as Table1.                                     ###
//###                                                               ###
//### To query for the values, we should select PSG's Register 14,  ###
//### which is done writting 0Eh (14) to 8255 PPI port A (which is  ###
//### directly connected to the PSG). Then, Bits 3..0 of PPI port C ###
//### are connected to a decoder that sends this to the keyboard,   ###
//### selecting Matrix Line to be read. Bits 7-6 are conected to    ###
//### PSG's operation mode selector, and lets us select between     ###
//### (00)inactive / (01)read / (10)write / (11)register_select     ###
//### operation modes. So, writting C0h (11 000000) to Port C we    ###
//### tell the PSG to select a register (the 0Eh that previouly was ###
//### send to PSG through port A). Then, it is possible to start    ###
//### asking for Matrix Lines and reading the Reg14 through Port A  ###
//### to get the pressed/not pressed values from the Matrix.        ###
//### Just one detail left: it is necessary to put PSG into inactive###
//### mode between different opperations.                           ###
//### Summing up:                                                   ###
//###     > 1: Configure PPI Operation Mode for:                    ###
//###          >> Port A: Ouput, Port C: Output (10000010b = 82h)   ###
//###     > 2: Write 14 (0Eh) to Port A (the index of the register) ###
//###     > 3: Write C0h to Port C (11 000000) to tell PSG that we  ###
//###          want to select a register (indexed at Port A).       ###
//###     > 4: Write 0 (00 000000) to Port C to finish operation    ###
//###         (put PSG inactive between different operations)       ###
//###     > 5: Configure PPI Operation Mode for:                    ###
//###          >> Port A: Input, Port C: Output (10010010b = 92h)   ###
//###     > 6: Write Matrix Line ID to Port C                       ###
//###     > 7: Read Matrix Line Status from Port A                  ###
//###     > 8: Repeat 6 until all Matrix Lines are read             ###
//###     > 9: Configure Again PPI as in (1) (82h Output/Output)    ###
//###          to leave it in this state.                           ###
//#####################################################################
//
// (Table1) MAPPING OF KEYBOARD LINES TO CONCRETE KEYS/SWITCHES
//=========================================================================================================
//|     |                                       L I N E                                                   |
//|     |-------------------------------------------------------------------------------------------------|
//| BIT |      0      |     1      |   2   |  3  |  4  |  5   |      6       |  7  |    8     |     9     |
//|=====|=============|============|=======|=====|=====|======|==============|=====|==========|===========|
//|  7  | f.          | f0         | Ctrl  | > , | < . | Space| V            | X   | Z        | Del       |
//|  6  | Enter       | f2         | ` \   | ? / | M   | N    | B            | C   | Caps Lock| Unused    |
//|  5  | f3          | f1         | Shift | * : | K   | J    | F Joy1_Fire1 | D   | A        | Joy0_Fire1|
//|  4  | f6          | f5         | f4    | + ; | L   | H    | G Joy1_Fire2 | S   | Tab      | Joy0_Fire2|
//|  3  | f9          | f8         | } ]   | P   | I   | Y    | T Joy1_Right | W   | Q        | Joy0_Right|
//|  2  | Cursor Down | f7         | Return| | @ | O   | U    | R Joy1_Left  | E   | Esc      | Joy0_Left |
//|  1  | Cursor Right| Copy       | { [   | = - | ) 9 | ' 7  | % 5 Joy1_Down| # 3 | " 2      | Joy0_Down |
//|  0  | Cursor Up   | Cursor Left| Clr   | £ ^ | _ 0 | ( 8  | & 6 Joy1_Up  | $ 4 | ! 1      | Joy0_Up   |
//=========================================================================================================
// Notes:
//   > Bit 6 on lines 9 and 6, may be used to report a third fire button on a joystick. This bit is also used as the middle button on an AMX compatible mouse.  
//   > "f." is the "." key on the numeric keypad. 
//   > Enter is the Small enter key, whereas Return is the large one.
//   > If matrix line 11-14 are selected, the byte is always &ff. After testing on a real CPC, it is found that these never change, they always return &FF. 
//


#ifndef CPCT_KEYBOARD_H
#define CPCT_KEYBOARD_H

#include <types.h>

///
/// Declare type for CPC keys
///
enum cpct_e_keyID;
typedef enum cpct_e_keyID cpct_keyID;

///
/// Function Declarations
///
extern void cpct_scanKeyboard     ();
extern void cpct_scanKeyboardFast ();
extern   u8 cpct_isKeyPressed     (cpct_keyID key);

///
/// KEY DEFINITIONS
/// Enumerated value with symbols for all the 80 possible Key/Joy definitions
///
enum cpct_e_keyID
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
};

#endif
