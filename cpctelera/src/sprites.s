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
;### MODULE: Sprites                                               ###
;#####################################################################
;### This module contains several functions and routines to manage ###
;### sprites and video memory in an Amstrad CPC environment.       ###
;#####################################################################
;

;
;########################################################################
;### FUNCTION: cpct_drawSprite2x8_aligned                             ###
;########################################################################
;### Copies a 2x8-bytes sprite from its original memory storage       ###
;### position to a video memory position (taking into account video   ###
;### memory distribution). This function asumes that the destination  ###
;### in the video memory will be the starting line of a character.    ###
;### (First byte line of video memory, where characters from the 25   ###
;###  lines start. First 2000 bytes, C000 to C7D0, in bank 4)         ###
;### It also asumes that the sprite is a solid entity and all of its  ###
;### bytes are stored consecutively: they are copied as they are.     ###
;########################################################################
;### INPUTS (4 Bytes)                                                 ###
;###  * (2B) Source Sprite Pointer (16-byte vector with pixel data)   ###
;###  * (2B) Destiny aligned video memory start location              ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURED TIME                                                    ###
;###  93 + 8 * 50 + 7 * 27 = 682 cycles                               ###
;########################################################################
;
.globl _cpct_drawSprite2x8_aligned
_cpct_drawSprite2x8_aligned::
  ;; GET Parameters from the stack (Push+Pop is faster than referencing with IX)
   POP  AF                  ;; [10c] AF = Return Address
   POP  HL                  ;; [10c] HL = Source address
   POP  DE                  ;; [10c] DE = Destination address
   PUSH DE                  ;; [11c] Leave the stack as it was
   PUSH HL                  ;; [11c] 
   PUSH AF                  ;; [11c] 

   ;; Copy 8 lines of 2 bytes width
   LD   BC, #16             ;; [10c] 16 bytes is the total we have to transfer (2x8)
   JP   dsa28_first_line    ;; [10c] First line does not need to do math to start transfering data. 

dsa28_next_line:
   LD   A, D                ;; [ 4c] Add 800h and Subtract 2h to DE (Jump to next visual line in the video memory)
   ADD  A, #8               ;; [ 7c]
   LD   D, A                ;; [ 4c]
   DEC  DE                  ;; [ 6c]
   DEC  DE                  ;; [ 6c]

dsa28_first_line:
   LDI                      ;; [16c] Copy 2 bytes with (DE) <- (HL) and decrement BC (distance is 1 byte less as we progress up)
   LDI                      ;; [16c]
   XOR  A                   ;; [ 4c] A = 0
   XOR  C                   ;; [ 4c] Check if C = 0 using XOR (as A is already 0)
   JP   NZ,dsa28_next_line  ;; [10c] 

   RET                      ;; [10c]

;
;########################################################################
;### FUNCTION: cpct_drawSprite4x8_aligned                             ###
;########################################################################
;### Copies a 4x8-bytes sprite from its original memory storage       ###
;### position to a video memory position (taking into account video   ###
;### memory distribution). This function asumes that the destination  ###
;### in the video memory will be the starting line of a character.    ###
;### (First byte line of video memory, where characters from the 25   ###
;###  lines start. First 2000 bytes, C000 to C7D0, in bank 4)         ###
;### It also asumes that the sprite is a solid entity and all of its  ###
;### bytes are stored consecutively: they are copied as they are.     ###
;########################################################################
;### INPUTS (4 Bytes)                                                 ###
;###  * (2B) Source Sprite Pointer (32-byte vector with pixel data)   ###
;###  * (2B) Destiny aligned video memory start location              ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURED TIME                                                    ###
;###  90 + 8 * 88 + 7 * 19 = 927 cycles                               ###
;########################################################################
;
.globl _cpct_drawSprite4x8_aligned
_cpct_drawSprite4x8_aligned::
   ;; GET Parameters from the stack (Push+Pop is faster than referencing with IX)
   POP  AF                 ;; [10c] AF = Return Address
   POP  HL                 ;; [10c] HL = Source address
   POP  DE                 ;; [10c] DE = Destination address
   PUSH DE                 ;; [11c] Leave the stack as it was
   PUSH HL                 ;; [11c] 
   PUSH AF                 ;; [11c] 

   ;; Copy 8 lines of 4 bytes width
   LD   A, #8              ;; [ 7c] We have to draw 8 lines of sprite
   JP   dsa48_first_line   ;; [10c] First line does not need to do math to start transfering data. 

dsa48_next_line:
   ;; Move to the start of the next line
   EX   DE, HL             ;; [ 4c] Make DE point to the start of the next line of pixels in video memory, by adding BC
   ADD  HL, BC             ;; [11c] (We need to interchange HL and DE because DE does not have ADD)
   EX   DE, HL             ;; [ 4c]

dsa48_first_line:
   ;; Draw a sprite-line of 4 bytes
   LD   BC, #0x800         ;; [10c] 800h bytes is the distance to the start of the first pixel in the next line in video memory (it will be decremented by 1 by each LDI)
   LDI                     ;; [16c] <|Copy 4 bytes with (DE) <- (HL) and decrement BC (distance is 1 byte less as we progress up)
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|

   ;; Repeat for all the lines
   DEC  A                  ;; [ 4c] A = A - 1 (1 more line completed)
   JP   NZ,dsa48_next_line ;; [10c] 

   RET                     ;; [10c]


;;
;;  WORK IN PROGRESS...
;;

; (Max sprite width=80 bytes)
;.globl _cpct_drawSprite
_cpct_drawSprite::
   ;; GET Parameters from the stack (Push+Pop is faster than referencing with IX)
   POP  AF                 ;; [10c] AF = Return Address
   POP  HL                 ;; [10c] HL = Source address
   POP  DE                 ;; [10c] DE = Destination address
   POP  BC                 ;; [10c] BC = Width/Height
   PUSH BC                 ;; [11c] Leave the stack as it was
   PUSH DE                 ;; [11c] 
   PUSH HL                 ;; [11c] 
   PUSH AF                 ;; [11c] 

   ;; B = Width, C = Height

   ;; Modify code using width to jump in drawSpriteWidth
   LD  A, #80                    ;; [ 7c]
   SUB B                         ;; [ 4c]
   LD (ds_drawSpriteWidth+#4), A ;; [13c]

   ;; Get the current pixel-row of the screen where we are painting (there are 8 pixel rows)
   ;; Pixel row are bits 11-13 of the address pointer (14-15 identify the page 0000/4000/8000/C000)
   ;;LD  A, D                ;; [ 4c]
   ;;RRCA                    ;; [ 4c]
   ;;RRCA                    ;; [ 4c]
   ;;RRCA                    ;; [ 4c]
   ;;NEG                     ;; [ 8c] Two's Complement
   ;;AND #0x07               ;; [ 7c] 07 = 00000111

   ;;LD  B, A                ;; [ 4c]
   ;;LD  A, C                ;; [ 4c]
   ;;SUB B                   ;; [ 4c]
   ;;.DW #0x67DD             ;; [ 8c] LD IXh, A (Save A = C - A)
   
   ;;LD  A, B                ;; [ 4c]
   
   LD   A, C                 ;; [ 4c] A = Height
   ;;JP  ds_drawSpriteWidth  ;; [10c]
   EX   DE, HL               ;; [ 4c]

ds_drawSpriteWidth_next:
   ;; NEXT LINE
   EX   DE, HL             ;; [ 4c]

ds_drawSpriteWidth:
   ;; Draw a sprite-line of n bytes
   LD BC, #0x800           ;; [10c]
   JR 0                    ;; [12c] Self modifying instruction: the 0 will be substituted by the required jump forward
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|

   DEC A                   ;; [ 4c]
   RET Z                   ;; [11c/5c]
   ;;JP Z, ds_drawEnds       ;; [10c]

   EX  DE, HL              ;; [ 4c]
   ADD HL, BC              ;; [11c]
   LD  A, D                ;; [ 4c]
   AND #0x38               ;; [ 7c]
   JP NZ, ds_drawSpriteWidth_next ;; [10c]

   LD  BC, #0xB850         ;; [10c] B850h = C050h - 800h
   ADD HL, BC              ;; [11c]
   JP ds_drawSpriteWidth_next ;; [10c]

ds_drawEnds:   
   RET                     ;; [10c]
   
C000
C800
D000 11 011100
D800 11 011100 BC-800 = -Width
E000 11 100000 BC-37B0
E800 11 101100
F000 11 110000
F800 11 111100 -37B0-Width

C050
- 3FB0