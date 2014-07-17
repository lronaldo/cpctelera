;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
;;
;;  This program is free software: you can redistribute it and/or modify
;;  it under the terms of the GNU General Public License as published by
;;  the Free Software Foundation, either version 3 of the License, or
;;  (at your option) any later version.
;;
;;  This program is distributed in the hope that it will be useful,
;;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;  GNU General Public License for more details.
;;
;;  You should have received a copy of the GNU General Public License
;;  along with this program.  If not, see <http://www.gnu.org/licenses/>.
;;-------------------------------------------------------------------------------
;#####################################################################
;### MODULE: Keyboard                                              ###
;#####################################################################
;### Routines control and check keyboard status, keys and joystick ###
;#####################################################################
;### TECHNICAL INFORMATION:                                        ###
;### Keyboard and joystick are connected to AY-3-8912 Programable  ###
;### Sound Generator (PSG) and the 8255 Programable Peripheral     ###
;### Interface (PPI) chips. To read keyboard status we have to     ###
;### communicate with these two chips using OUT command.           ###
;###                                                               ###
;### Keyboard and joystick switches and buttons are arranged in a  ###
;### 10x8 matrix. Each element of the matrix represents the state  ###
;### of one button/switch/key (pressed/not pressed). That means    ###
;### the CPC can control up to 80 keys/switches/buttons in total.  ###
;###                                                               ###
;### We're able to read a complete column of the matrix each time. ###
;### That means we get the state of 8 switches at a time, in the   ###
;### form of a byte (each bit represents the state of an switch).  ###
;### Each bit will hold a "0" if the switch is "pressed" or a "1"  ###
;### when the switch is "not pressed".                             ###
;###                                                               ###
;### It is relevant to notice something about joysticks. Although  ###
;### joystick 0 has its own column in the matrix (9th) to control  ###
;### its 6 switches, joystick 1 shares its switches with other     ###
;### keys in the 6th column (namely: F, G, T, R, 5, 6). Therefore, ###
;### it is possible to emulate a 2nd joystick using keyboard. The  ###
;### exact mapping between matrix values and switches pressed is   ###
;### included below as Table1.                                     ###
;###                                                               ###
;### To query for the values, you should address Port A from PSG   ###
;### (Using Register 14 from PSG). Bits 3..0 of PPI port C are     ###
;### used to select the matrix line to be read. The data of the    ###
;### selected line will then be presente at PSG Port A (R14).      ###
;###                                                               ###
;### Amstrad CPC 8255 PPI Port Reference:                          ###
;###  > PPI Port A           = #F4xx                               ###
;###  > PPI Port B           = #F5xx                               ###
;###  > PPI Port C           = #F6xx                               ###
;###  > PPI Control-Register = #F7xx                               ###
;#####################################################################
;

; (Table1) MAPPING OF KEYBOARD LINES TO CONCRETE KEYS/SWITCHES
;=========================================================================================================
;|     |                                       L I N E                                                   |
;|     |-------------------------------------------------------------------------------------------------|
;| BIT |      0      |     1      |   2   |  3  |  4  |  5   |      6       |  7  |    8     |     9     |
;|=====|=============|============|=======|=====|=====|======|==============|=====|==========|===========|
;|  7  | f.          | f0         | Ctrl  | > , | < . | Space| V            | X   | Z        | Del       |
;|  6  | Enter       | f2         | ` \   | ? / | M   | N    | B            | C   | Caps Lock| Unused    |
;|  5  | f3          | f1         | Shift | * : | K   | J    | F Joy1_Fire1 | D   | A        | Joy0_Fire1|
;|  4  | f6          | f5         | f4    | + ; | L   | H    | G Joy1_Fire2 | S   | Tab      | Joy0_Fire2|
;|  3  | f9          | f8         | } ]   | P   | I   | Y    | T Joy1_Right | W   | Q        | Joy0_Right|
;|  2  | Cursor Down | f7         | Return| | @ | O   | U    | R Joy1_Left  | E   | Esc      | Joy0_Left |
;|  1  | Cursor Right| Copy       | { [   | = - | ) 9 | ' 7  | % 5 Joy1_Down| # 3 | " 2      | Joy0_Down |
;|  0  | Cursor Up   | Cursor Left| Clr   | £ ^ | _ 0 | ( 8  | & 6 Joy1_Up  | $ 4 | ! 1      | Joy0_Up   |
;=========================================================================================================
; Notes:
;   > Bit 6 on lines 9 and 6, may be used to report a third fire button on a joystick. This bit is also used as the middle button on an AMX compatible mouse.  
;   > "f." is the "." key on the numeric keypad. 
;   > Enter is the Small enter key, whereas Return is the large one.
;   > If matrix line 11-14 are selected, the byte is always &ff. After testing on a real CPC, it is found that these never change, they always return &FF. 
;

;
;########################################################################
;### FUNCTION: _cpct_readFullKeyboardStatus                           ###
;########################################################################
;###  ###
;########################################################################
;### INPUTS (-)                                                       ###
;########################################################################
;### OUTPUTS (-)                                                      ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, HL                           ###
;########################################################################
;### MEASURED TIME                                                    ###
;###  ?? cycles                                                       ###
;########################################################################
;### CREDITS:                                                         ###
;########################################################################
;
; Define a 10-byte buffer to store keyboard data
keyboardBuffer: .ds 10

.globl _cpct_readFullKeyboardStatus
_cpct_readFullKeyboardStatus::
    
    DI
    LD HL, #keyboardBuffer    ;; [] HL Points to the start of the keyboardBuffer, where scanned data will be stored
    
    LD BC, #0xF782            ;; [] Configure PPI 8255: Set Both Port A and Port C as Output
    OUT (C), C                ;; []
    
    LD BC, #0xF40E            ;; [] Select AY-3-8912 Register 14 (0E) on PPI 8255 Port A (F4)
    LD E,  B                  ;; []
    OUT (C), C                ;; []
    
    LD BC, #0xF6C0            ;; [] 
    LD D,  B                  ;; []
    OUT (C), C                ;; []

    LD C,  #0x00
    OUT (C), C                ;; []

    LD BC, #0xf792
    OUT (C), C                ;; []

    LD A, #0x40
    LD C, #0x4a
    
rfks_nextKeyboardLine:
    LD B, D
    OUT (C), A                ;; []
    
    LD B, E
    INI
    INC A
    CP C
    JR C,rfks_nextKeyboardLine
    
    LD BC, #0xF782
    OUT (C), C                ;; []
    EI
    
    RET