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
;###  * (1B C) Box Width in bytes (Max. 63 bytes)                     ###
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
   dec c                          ;; [ 4] The first line of bytes has 1 byte less to be copied (the first value we have already copied)
   ld  a, #126                    ;; [ 7] We need to jump 126 bytes (63 LDIs*2 bytes) minus the width of the sprite*2 (2B)
   sub c                          ;; [ 4]    to do as much LDIs as bytes the Sprite is wide
   sub c                          ;; [ 4]
   ld (dsb_drawSpriteWidth+#4), A ;; [13] Modify JR data to create the jump we need

dsb_drawSpriteWidth:
   ;; Draw a sprite-line of n bytes /// TODO: This is not to be done first time
   ld   a, b               ;; [ 4] A = Height (used as counter for the number of lines we have to copy)
   push de   ; 11
   ldi       ; 16
   .DW #0x0018  ;; JR 0    ;; [12] Self modifying instruction: the '00' will be substituted by the required jump forward.
                           ;;      (Note: Writting JR 0 compiles but later it gives odd linking errors)
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

   ;; Move destination pointer to
   ld b, a   ; 4 save A into b
   pop hl    ; 10
   ld e, l   ; 4
   ld a, h   ; 4
   and #0x38 ; 7
   jp z, dsb_next8block ; 10

   ld a, h   ; 4
   add #8    ; #7
   ld d, a   ; 4
   jp dsb_drawSpriteWidth ; 10
dsb_next8block:
   ;; Jump destination pointer to the start of the next line in video memory
   ld   d, h       ; 4
   ld  bc, #0xC050 ; 10
   add hl, bc      ; [11] We add 0x800 minus the width of the sprite (BC) to destination pointer
   ex  de, hl      ; 4
   jp dsb_drawSpriteWidth ;; [10] If that does not leave as with 0, we are still inside video memory boundaries, so proceed with next line