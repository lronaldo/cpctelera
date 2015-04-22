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
;### INPUTS (5 Bytes)                                                 ###
;###  * (2B DE) Video memory pointer to the upper left box corner byte###
;###  * (1B)   1-byte colour pattern to fill the box with             ###
;###  * (1B C) Box Width in bytes                                     ###
;###  * (1B B) Box Height in bytes (Max. 64 bytes)                    ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURES                                                         ###
;### MEMORY:  bytes                                                   ###
;### TIME:                  (w=width, h=height)                       ###
;########################################################################
;

_cpct_drawSolidBox::
   ;; GET Parameters from the stack
   ld  hl, #2         ;; [10] HL Points to SP+2 (first 2 bytes are return address)
   add hl, sp         ;; [11]    , to use it for getting parameters from stack
   ld   e, (hl)       ;; [ 7] DE = First Parameter (Video memory pointer)
   inc hl             ;; [ 6]
   ld   d, (hl)       ;; [ 7]
   inc hl             ;; [ 6]  / Copy first value to video memory (upper-left corner of the box)
   ldi                ;; [16] (HL)->(DE) Move second parameter (1-byte Colour Pattern) directly into first byte of the box in memory
   ld   c, (hl)       ;; [ 7] C = Third Parameter (Box Width)
   inc hl             ;; [ 6]
   ld   b, (hl)       ;; [ 7] B = Fourth Parameter (Box Height)

   ;; Prepare HL and DE pointers for copying bytes consecutively
   ld   h, d          ;; [ 4] HL = DE - 1 (HL Points to the first byte of the box, the one that contains the colour pattern)
   ld   l, e          ;; [ 4]
   dec  hl            ;; [ 6]
   push hl            ;; [11] Save HL (Pointer to the first byte of the box) for later use

   ;; Modify code using width to jump in drawSpriteWidth
   dec c                       ;; [ 4] The first line of bytes has 1 byte less to be copied (the first value we have already copied)
   ld  a, #126                 ;; [ 7] We need to jump 126 bytes (63 LDIs*2 bytes) minus the width of the sprite*2 (2B)
   sub c                       ;; [ 4]    to do as much LDIs as bytes the Sprite is wide
   sub c                       ;; [ 4]
   ld (dsb_drawFirstLine+3), A ;; [13] Modify JR data to create the jump we need
   jp dsb_drawFirstLine        ;; [10] Jump to the code required for the first line (different from next lines)

   ;; Draw a sprite-line of n bytes 
dsb_drawSpriteWidth:
   push de              ;; [11] Save DE pointer, at the start of this next line to be copied
   ldi                  ;; [16] Copy the first byte, as next code is configured for width-1 bytes
dsb_drawFirstLine:
   ld c, #0xFF          ;; [ 7] C = 255 to ensure B never gets modified by LDIs (that discount 1 from BC)
   .DW #0x0018  ;; JR 0 ;; [12] Self modifying instruction: the '00' will be substituted by the required jump forward.
                        ;;      (Note: Writting JR 0 compiles but later it gives odd linking errors)
   ldi                  ;; [16] <| 63 ldis, which are able to copy up to 63 bytes each time.
   ldi                  ;; [16]  | That means that each Sprite line should be 63 bytes width at most.
   ldi                  ;; [16]  | The JR instruction at the start makes us ingnore the ldis we dont need (jumping over them)
   ldi                  ;; [16] <| That ensures we will be doing only as much ldis as bytes our sprite is wide
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16]  |
   ldi                  ;; [16] <|
   ldi                  ;; [16] <|
   ldi                  ;; [16]  |
   ldi                  ;; [16]  |

   pop hl               ;; [10] HL = Pointer to the start of the last completed line (was saved previously with push de)

   dec b                ;; [ 4] Another line finished: we discount it from B (Height)
   ret z                ;; [11/5] If that was the last line, we safely return

   ;; Set up HL and DE pointers for copying next line of bytes
   ;; HL will point to the start of the last completed line and DE to the start of the next

   ;; Next pixel line is +0x800 bytes away (7 out of every 8 times)
   ld   e, l                   ;; [ 4] To make DE = HL (previous to incrementing it to the next line), we first copy E = L
   ld   a, h                   ;; [ 4] D = H + 0x08 (Add 0x800 to DE, which is equal to HL, because E = L)
   add #8                      ;; [ 7]
   ld   d, a                   ;; [ 4]
                               ;; Check if DE (next line) is first line of a screen character (8-byte aligned)...
   and #0x38                   ;; [ 7] ...by checking bits xx000xxxx. If the 3 of them are 0, it is the start of a new...
   jp  nz,dsb_drawSpriteWidth  ;; [10] ...screen character, and next pixel line is -0xFB0 (+0xC050) bytes away... 
                               ;;      ...else, DE already points to next pixel line, with previoys +0x800 bytes added.      
                               
   ;; Next pixel line is -0xFB0 (+0xC050) bytes away
dsb_next8block:
   ld   a, e              ;; [ 4] DE = DE + 0xC050 (DE Points to next pixel line)
   add #0x50              ;; [ 7]
   ld   e, a              ;; [ 4]
   ld   a, d              ;; [ 4]
   adc #0xC0              ;; [ 7]
   ld   d, a              ;; [ 4]
   jp dsb_drawSpriteWidth ;; [10] Draw next pixel line