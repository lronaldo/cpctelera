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
;### FUNCTION: cpct_drawSprite2x8Fast_aligned                         ###
;########################################################################
;### Does the same as cpct_drawSprite2x8_aligned, but using an unro-  ###
;### lled version of the loop, which is ~27% faster. The only draw-   ###
;### back is that this version requires more memory space to store    ###
;### the code (+173%).                                                ###
;########################################################################
;### INPUTS (4 Bytes)                                                 ###
;###  * (2B) Source Sprite Pointer (16-byte vector with pixel data)   ###
;###  * (2B) Destiny aligned video memory start location              ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURES                                                         ###
;###  MEMORY:  71 bytes                                               ###
;###  TIME:   449 cycles (113.25 us)                                  ###
;########################################################################
;

_cpct_drawSpriteAligned2x8_f::
  ;; GET Parameters from the stack (Push+Pop is faster than referencing with IX)
   pop  af                  ;; [10] AF = Return Address
   pop  hl                  ;; [10] HL = Source address
   pop  de                  ;; [10] DE = Destination address
   push de                  ;; [11] Leave the stack as it was
   push hl                  ;; [11]
   push af                  ;; [11]

   ;; Copy 8 lines of 2 bytes width (2x8 = 16 bytes)
   ;;  (Unrolled version of the loop)
   ld    a, d               ;; [ 4] First, save DE into A and B, 
   ld    b, e               ;; [ 4]   to ease the 800h increment step
   ld    c, #17             ;; [ 7] Ensure that 16 LDIs do not change value of B (as they will decrement BC)

   ;; Sprite Line 1
   ldi                      ;; [16] Copy line (2 bytes)
   ldi                      ;; [16]
   add   #8                 ;; [ 7] DE += 800h (Using previous A, B copy)
   ld    d, a               ;; [ 4]
   ld    e, b               ;; [ 4]

   ;; Sprite Line 2
   ldi                      ;; [16] Copy line (2 bytes)
   ldi                      ;; [16]
   add   #8                 ;; [ 7] DE += 800h (Using previous A, B copy)
   ld    d, a               ;; [ 4]
   ld    e, b               ;; [ 4]

   ;; Sprite Line 3
   ldi                      ;; [16] Copy line (2 bytes)
   ldi                      ;; [16]
   add   #8                 ;; [ 7] DE += 800h (Using previous A, B copy)
   ld    d, a               ;; [ 4]
   ld    e, b               ;; [ 4]

   ;; Sprite Line 4
   ldi                      ;; [16] Copy line (2 bytes)
   ldi                      ;; [16]
   add   #8                 ;; [ 7] DE += 800h (Using previous A, B copy)
   ld    d, a               ;; [ 4]
   ld    e, b               ;; [ 4]

   ;; Sprite Line 5
   ldi                      ;; [16] Copy line (2 bytes)
   ldi                      ;; [16]
   add   #8                 ;; [ 7] DE += 800h (Using previous A, B copy)
   ld    d, a               ;; [ 4]
   ld    e, b               ;; [ 4]

   ;; Sprite Line 6
   ldi                      ;; [16] Copy line (2 bytes)
   ldi                      ;; [16]
   add   #8                 ;; [ 7] DE += 800h (Using previous A, B copy)
   ld    d, a               ;; [ 4]
   ld    e, b               ;; [ 4]

   ;; Sprite Line 7
   ldi                      ;; [16] Copy line (2 bytes)
   ldi                      ;; [16]
   add   #8                 ;; [ 7] DE += 800h (Using previous A, B copy)
   ld    d, a               ;; [ 4]
   ld    e, b               ;; [ 4]

   ;; Sprite Line 8
   ldi                      ;; [16] Copy line (2 bytes)
   ldi                      ;; [16]

   ret                      ;; [10]
