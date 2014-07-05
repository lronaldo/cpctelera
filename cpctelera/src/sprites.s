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
;######################################################################
;### FUNCTION: cpct_drawSprite2x8_aligned                           ###
;######################################################################
;### Copies a 2x8-bytes sprite from its original memory storage     ###
;### position to a video memory position (taking into account video ###
;### memory distribution). This function asumes that the destination###
;### in the video memory will be the starting line of a character.  ###
;### (First byte line of video memory, where characters from the 25 ###
;###  lines start)                                                  ###
;######################################################################
;### INPUTS (4 Bytes)                                               ###
;###  * (2B) Original Sprite Pointer                                ###
;###  * (2B) Destiny aligned video memory location (first VDU line) ###
;######################################################################
;### EXIT STATUS                                                    ###
;###  Destroyed Register values: AF, BC, DE, HL                     ###
;######################################################################
;
.globl _cpct_drawSprite2x8_aligned
_cpct_drawSprite2x8_aligned::
  ;; Save registers that should be restored
  PUSH IX
 
  ;; GET Parameters from the stack using IX
  LD  IX, #4	;; 4 = 2B (IX) + 2B (Return Address)
  ADD IX, SP
  LD  L, 0(IX)	;; HL = Original sprite pointer
  LD  H, 1(IX)  ;; DE = Destiny Aligned video memory
  LD  E, 2(IX)
  LD  D, 3(IX)

  ;; Copy 8 lines of 2 bytes width
  LD  A,  #8
  LD  BC, #0x7FE	;; 800h - 2 = 7FEh (Quantity to jump to next line)
dsa_next_line:
  LDI			;; Copy 2 bytes with (DE) <- (HL)
  LDI
  ADD HL, BC		;; Move HL to the start point of next line in video memory
  DEC A			;; 1 less line to go
  JP NZ, dsa_next_line
  
  ;; Restore saved registers and return
  POP IX
  RET
