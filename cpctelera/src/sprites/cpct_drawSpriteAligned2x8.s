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
;### FUNCTION: cpct_drawSpriteAligned2x8                              ###
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
_cpct_drawSpriteAligned2x8::
  ;; GET Parameters from the stack (Push+Pop is faster than referencing with IX)
   pop  af                  ;; [10] AF = Return Address
   pop  hl                  ;; [10] HL = Source address
   pop  de                  ;; [10] DE = Destination address
   push de                  ;; [11] Leave the stack as it was
   push hl                  ;; [11]
   push af                  ;; [11]

   ;; Copy 8 lines of 2 bytes width (2x8 = 16 bytes)
   ld    a, d               ;; [ 4] Save D into A for fastly doing +800h increment of DE
   ld   bc, #16             ;; [10] BC = 16, countdown of the number of bytes remaining
   jp   dsa28_first_line    ;; [10] First line does not need to do math to start transfering data. 

dsa28_next_line:
   ;; This 4 lines do "DE += 800h - 2h" to move DE pointer to the next pixel line at video memory
   dec   e                  ;; [ 4] E -= 2 (We only decrement E because D is saved into A, 
   dec   e                  ;; [ 4]         hence, it does not matter if E is 00 and becomes FF, cause we do not have to worry about D)
   add   #8                 ;; [ 7] D += 8 (To add 800h to DE, we increment previous value of D by 8, and move it into D)
   ld    d, a               ;; [ 4]

dsa28_first_line:
   ldi                      ;; [16] Copy 2 bytes for (HL) to (DE) and decrement BC 
   ldi                      ;; [16]
   jp   pe, dsa28_next_line ;; [10] While BC!=0, continue copying bytes (When BC=0 LDI resets P/V, otherwise, P/V is set)

   ret                      ;; [10]
