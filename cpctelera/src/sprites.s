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
;###  lines start)                                                    ###
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
  POP  AF			;; [10c] AF = Return Address
  POP  HL			;; [10c] HL = Source address
  POP  DE			;; [10c] DE = Destination address
  PUSH DE			;; [11c] Leave the stack as it was
  PUSH HL			;; [11c] 
  PUSH AF			;; [11c] 
  
  ;; Copy 8 lines of 2 bytes width
  LD   BC, #16			;; [10c] 16 bytes is the total we have to transfer (2x8)
  JP   dsa28_first_line		;; [10c] First line does not need to do math to start transfering data. 

dsa28_next_line:
  LD   A, D			;; [ 4c] Add 800h and Subtract 2h to DE (Jump to next visual line in the video memory)
  ADD  A, #8			;; [ 7c]
  LD   D, A			;; [ 4c]
  DEC  DE			;; [ 6c]
  DEC  DE			;; [ 6c]

dsa28_first_line:
  LDI				;; [16c] Copy 2 bytes with (DE) <- (HL) and decrement BC (distance is 1 byte less as we progress up)
  LDI				;; [16c]
  XOR  A			;; [ 4c] A = 0
  XOR  C			;; [ 4c] Check if C = 0 using XOR (as A is already 0)
  JP   NZ,dsa28_next_line 	;; [10c] 
  
  ;; RETURN
  RET			;; [10c]
