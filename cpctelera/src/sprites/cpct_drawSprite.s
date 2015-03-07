;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

;
;########################################################################
;### FUNCTION: cpct_drawSprite                                        ###
;########################################################################
;### This function copies a generic NxM bytes sprite from memory to a ###
;### video-memory location (either present video-memory or hardware   ###
;### backbuffer. The original sprite must be stored as an array (i.e. ###
;### with all of its pixels stored as consecutive bytes in memory) It ###
;### only works for solid, rectangular sprites, with 1-63 bytes width ###
;########################################################################
;### INPUTS (6 Bytes)                                                 ###
;###  * (2B HL) Source Sprite Pointer (array with pixel data)         ###
;###  * (2B DE) Destination video memory pointer                      ###
;###  * (1B B) Sprite Height in bytes                                 ###
;###  * (1B C) Sprite Width in bytes (Max.63)                         ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURES                                                         ###
;### MEMORY: 191 bytes                                                ###
;### TIME:                  (w=width, h=height)                       ###
;###  - Best Case:  76 + (79 + 16w)h + 31(|h/8| - 1) cycles;          ###
;###  - Worst Case: Best Case + 31 cycles                             ###
;###  ** EXAMPLES **                                                  ###
;###   - 2x16 bytes sprite = 1883 / 1914 cycles ( 470.75 /  478.5 us) ###
;###   - 4x32 bytes sprite = 4745 / 4776 cycles (1186.25 / 1194.0 us) ###
;########################################################################
;
.globl _cpct_drawSprite
_cpct_drawSprite::
   ;; GET Parameters from the stack (Pop is fastest way)
   LD (ds_restoreSP+1), SP    ;; [20] Save SP into placeholder of the instruction LD SP, 0, to quickly restore it later.
   DI                         ;; [ 4] Disable interrupts to ensure no one overwrites return address in the stack
   POP  AF                    ;; [10] AF = Return Address
   POP  HL                    ;; [10] HL = Source Address (Sprite data array)
   POP  DE                    ;; [10] DE = Destination address (Video memory location)
   POP  BC                    ;; [10] BC = Height/Width (B = Height, C = Width)
ds_restoreSP:
   LD SP, #0                  ;; [10] -- Restore Stack Pointer -- (0 is a placeholder which is filled up with actual SP value previously)
   EI                         ;; [ 4] Enable interrupts again

   ;; Modify code using width to jump in drawSpriteWidth
   LD  A, #126                   ;; [ 7] We need to jump 126 bytes (63 LDIs*2 bytes) minus the width of the sprite*2 (2B)
   SUB C                         ;; [ 4]    to do as much LDIs as bytes the Sprite is wide
   SUB C                         ;; [ 4]
   LD (ds_drawSpriteWidth+#4), A ;; [13] Modify JR data to create the jump we need

   LD   A, B                 ;; [ 4] A = Height (used as counter for the number of lines we have to copy)
   EX   DE, HL               ;; [ 4] Instead of jumping over the next line, we do the inverse operation because it is only 4 cycles and not 10, as a JP would be)

ds_drawSpriteWidth_next:
   ;; NEXT LINE
   EX   DE, HL             ;; [ 4] HL and DE are exchanged every line to do 16bit math with DE. This line reverses it before proceeding to copy the next line.

ds_drawSpriteWidth:
   ;; Draw a sprite-line of n bytes
   LD BC, #0x800           ;; [10] 0x800 bytes is the distance in memory from one pixel line to the next within every 8 pixel lines. Each LDI performed will decrease this by 1, as we progress through memory copying the present line
   .DW #0x0018  ;; JR 0    ;; [12] Self modifying instruction: the '00' will be substituted by the required jump forward. (Note: Writting JR 0 compiles but later it gives odd linking errors)
   LDI                     ;; [16] <| 80 LDIs, which are able to copy up to 80 bytes each time.
   LDI                     ;; [16]  | That means that each Sprite line should be 80 bytes width at most.
   LDI                     ;; [16]  | The JR instruction at the start makes us ingnore the LDIs we dont need (jumping over them)
   LDI                     ;; [16] <| That ensures we will be doing only as much LDIs as bytes our sprit is wide
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
   LD  BC, #0xC050         ;; [10] B850h = C050h - 800h
   ADD HL, BC              ;; [11] We advance destination pointer to next line
   JP ds_drawSpriteWidth_next ;; [10] and then continue copying
