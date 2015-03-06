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
.module cpct_drawSprite2x8Fast_aligned

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
.globl _cpct_drawSprite2x8Fast_aligned
_cpct_drawSprite2x8Fast_aligned::
  ;; GET Parameters from the stack (Push+Pop is faster than referencing with IX)
   POP  AF                  ;; [10] AF = Return Address
   POP  HL                  ;; [10] HL = Source address
   POP  DE                  ;; [10] DE = Destination address
   PUSH DE                  ;; [11] Leave the stack as it was
   PUSH HL                  ;; [11]
   PUSH AF                  ;; [11]

   ;; Copy 8 lines of 2 bytes width (2x8 = 16 bytes)
   ;;  (Unrolled version of the loop)
   LD   A, D                ;; [ 4] First, save DE into A and B, 
   LD   B, E                ;; [ 4]   to ease the 800h increment step
   LD   C, #17              ;; [ 7] Ensure that 16 LDIs do not change value of B (as they will decrement BC)

   ;; Sprite Line 1
   LDI                      ;; [16] Copy line (2 bytes)
   LDI                      ;; [16]
   ADD  #8                  ;; [ 7] DE += 800h (Using previous A, B copy)
   LD   D, A                ;; [ 4]
   LD   E, B                ;; [ 4]

   ;; Sprite Line 2
   LDI                      ;; [16] Copy line (2 bytes)
   LDI                      ;; [16]
   ADD  #8                  ;; [ 7] DE += 800h (Using previous A, B copy)
   LD   D, A                ;; [ 4]
   LD   E, B                ;; [ 4]

   ;; Sprite Line 3
   LDI                      ;; [16] Copy line (2 bytes)
   LDI                      ;; [16]
   ADD  #8                  ;; [ 7] DE += 800h (Using previous A, B copy)
   LD   D, A                ;; [ 4]
   LD   E, B                ;; [ 4]

   ;; Sprite Line 4
   LDI                      ;; [16] Copy line (2 bytes)
   LDI                      ;; [16]
   ADD  #8                  ;; [ 7] DE += 800h (Using previous A, B copy)
   LD   D, A                ;; [ 4]
   LD   E, B                ;; [ 4]

   ;; Sprite Line 5
   LDI                      ;; [16] Copy line (2 bytes)
   LDI                      ;; [16]
   ADD  #8                  ;; [ 7] DE += 800h (Using previous A, B copy)
   LD   D, A                ;; [ 4]
   LD   E, B                ;; [ 4]

   ;; Sprite Line 6
   LDI                      ;; [16] Copy line (2 bytes)
   LDI                      ;; [16]
   ADD  #8                  ;; [ 7] DE += 800h (Using previous A, B copy)
   LD   D, A                ;; [ 4]
   LD   E, B                ;; [ 4]

   ;; Sprite Line 7
   LDI                      ;; [16] Copy line (2 bytes)
   LDI                      ;; [16]
   ADD  #8                  ;; [ 7] DE += 800h (Using previous A, B copy)
   LD   D, A                ;; [ 4]
   LD   E, B                ;; [ 4]

   ;; Sprite Line 8
   LDI                      ;; [16] Copy line (2 bytes)
   LDI                      ;; [16]

   RET                      ;; [10]
