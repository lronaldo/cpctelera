;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_sprites

.include /sprites.s/

;
;########################################################################
;### FUNCTION: cpct_drawSolidBox                                      ###
;########################################################################
;### This function draws a solid 1 colour pattern box (a square full  ###
;### of a given colour pattern) anywhere at the screen.               ###
;########################################################################
;### INPUTS (6 Bytes)                                                 ###
;###  * (2B DE) Video memory pointer to the upper left box corner byte###
;###  * (1B B) Box Height in bytes                                    ###
;###  * (1B C) Box Width in bytes                                     ###
;###  * (2B A) 1-byte colour pattern to fill the box with             ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURES                                                         ###
;### MEMORY:  bytes                                                   ###
;### TIME:                  (w=width, h=height)                       ###
;########################################################################
;

.globl _cpct_drawSolidBox
_cpct_drawSolidBox::
   ;; GET Parameters from the stack
   ld  hl, #2         ;; [10] HL Points to SP+2 (first 2 bytes are return address)
   add sp, hl         ;; [11]    , to use it for getting parameters from stack
   ld   e, (hl)       ;; [ 7] DE = First Parameter (Video memory pointer)
   inc hl             ;; [ 6]
   ld   d, (hl)       ;; [ 7]
   inc hl             ;; [ 6]
   ld   b, (hl)       ;; [ 7] C = Second Parameter (Box Width)
   inc hl             ;; [ 6]
   ld   c, (hl)       ;; [ 7] B = Third Parameter (Box Height)
   inc hl             ;; [ 6]
   ldi                ;; [16] (HL)->(DE) Move third parameter (1-byte Colour Pattern) directly into first byte of the box in memory

   ;; Copy first value
   ld   h, d          ;; [ 4] HL = DE - 1 (HL Points to the first byte of the box, the one that contains the colour pattern)
   ld   l, e          ;; [ 4]
   dec  hl            ;; [ 6]

   ;; Modify code using width to jump in drawSpriteWidth
   dec c                          ;; [ 4] The first line of bytes has 1 byte less (the first value we have already copied)
   ld  a, #126                    ;; [ 7] We need to jump 126 bytes (63 LDIs*2 bytes) minus the width of the sprite*2 (2B)
   sub c                          ;; [ 4]    to do as much LDIs as bytes the Sprite is wide
   sub c                          ;; [ 4]
   ld (dsb_drawSpriteWidth+#4), A ;; [13] Modify JR data to create the jump we need

   ld   a, b                  ;; [ 4] A = Height (used as counter for the number of lines we have to copy)
   ex   de, hl                ;; [ 4] Instead of jumping over the next line, we do the inverse operation because it is only 4 cycles and not 10, as a JP would be)

ds_drawSpriteWidth_next:
   ;; NEXT LINE
   ex   de, hl                ;; [ 4] HL and DE are exchanged every line to do 16bit math with DE. This line reverses it before proceeding to copy the next line.

dsb_drawSpriteWidth:
   ;; Draw a sprite-line of n bytes
   LD BC, #0x800           ;; [10] 0x800 bytes is the distance in memory from one pixel line to the next within every 8 pixel lines. Each LDI performed will decrease this by 1, as we progress through memory copying the present line
   .DW #0x0018  ;; JR 0    ;; [12] Self modifying instruction: the '00' will be substituted by the required jump forward. (Note: Writting JR 0 compiles but later it gives odd linking errors)
   LDI                     ;; [16] <| 63 LDIs, which are able to copy up to 63 bytes each time.
   LDI                     ;; [16]  | That means that each Sprite line should be 63 bytes width at most.
   LDI                     ;; [16]  | The JR instruction at the start makes us ingnore the LDIs we dont need (jumping over them)
   LDI                     ;; [16] <| That ensures we will be doing only as much LDIs as bytes our sprite is wide
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |

   DEC A                   ;; [ 4] Another line finished: we discount it from A
   RET Z                   ;; [11c/5] If that was the last line, we safely return

   ;; Jump destination pointer to the start of the next line in video memory
   EX  DE, HL              ;; [ 4] DE has destination, but we have to exchange it with HL to be able to do 16bit math
   ADD HL, BC              ;; [11] We add 0x800 minus the width of the sprite (BC) to destination pointer 
   LD  B, A                ;; [ 4] Save A into B (B = A)
   LD  A, H                ;; [ 4] We check if we have crossed video memory boundaries (which will happen every 8 lines). If that happens, bits 13,12 and 11 of destination pointer will be 0
   AND #0x38               ;; [ 7] leave out only bits 13,12 and 11
   LD  A, B                ;; [ 4] Restore A from B (A = B)
   JP NZ, ds_drawSpriteWidth_next ;; [10] If that does not leave as with 0, we are still inside video memory boundaries, so proceed with next line

   ;; Every 8 lines, we cross the 16K video memory boundaries and have to
   ;; reposition destination pointer. That means our next line is 16K-0x50 bytes back
   ;; which is the same as advancing 48K+0x50 = 0xC050 bytes, as memory is 64K 
   ;; and our 16bit pointers cycle over it
   LD  BC, #0xC050            ;; [10] We advance destination pointer to next line
   ADD HL, BC                 ;; [11]  HL += 0xC050
   JP ds_drawSpriteWidth_next ;; [10] Continue copying
