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
;### MEASURES                                                         ###
;###  MEMORY:  25 bytes                                               ###
;###  TIME:   566 cycles (141.50 us)                                  ### 
;########################################################################
;
.globl _cpct_drawSprite2x8_aligned
_cpct_drawSprite2x8_aligned::
  ;; GET Parameters from the stack (Push+Pop is faster than referencing with IX)
   POP  AF                  ;; [10] AF = Return Address
   POP  HL                  ;; [10] HL = Source address
   POP  DE                  ;; [10] DE = Destination address
   PUSH DE                  ;; [11] Leave the stack as it was
   PUSH HL                  ;; [11]
   PUSH AF                  ;; [11]

   ;; Copy 8 lines of 2 bytes width (2x8 = 16 bytes)
   LD   A, D                ;; [ 4] Save D into A for fastly doing +800h increment of DE
   LD   BC, #16             ;; [10] BC = 16, countdown of the number of bytes remaining
   JP   dsa28_first_line    ;; [10] First line does not need to do math to start transfering data. 

dsa28_next_line:
   ;; This 4 lines do "DE += 800h - 2h" to move DE pointer to the next pixel line at video memory
   DEC  E                   ;; [ 4] E -= 2 (We only decrement E because D is saved into A, 
   DEC  E                   ;; [ 4]         hence, it does not matter if E is 00 and becomes FF, cause we do not have to worry about D)
   ADD  #8                  ;; [ 7] D += 8 (To add 800h to DE, we increment previous value of D by 8, and move it into D)
   LD   D, A                ;; [ 4]

dsa28_first_line:
   LDI                      ;; [16] Copy 2 bytes for (HL) to (DE) and decrement BC 
   LDI                      ;; [16]
   JP   PE, dsa28_next_line ;; [10] While BC!=0, continue copying bytes (When BC=0 LDI resets P/V, otherwise, P/V is set)

   RET                      ;; [10]
