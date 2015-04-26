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
.module cpct_memset

.include /sprites.s/

;
;########################################################################
;### FUNCTION: cpct_memset                                            ###
;########################################################################
;### Sets all the bytes of an array in memory to the same given value ###
;### This is the technique it uses:                                   ###
;###  - It first sets up the first byte of the array to the value     ###
;###  - Makes HL point to the first byte and DE to the second         ###
;###  - BC has the total bytes to copy minus 1 (the first already set)###
;###  - LDIR copies first byte into second, then second into third... ###
;### WARNING: This works only for values of BC > 1. Never use this    ###
;### function with values 0 or 1 as number of bytes to copy, as it    ###
;### will have unexpected results (In fact, it could write up the en- ###
;### tire memory).                                                    ###
;########################################################################
;### INPUTS (1 Byte)                                                  ###
;###  * (2B DE) Starting point in memory                              ###
;###  * (2B BC) Number of bytes to be set (should be greater than 1!) ###
;###  * (1B A ) Value to be set                                       ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURES                                                         ###
;### MEMORY:  20 bytes                                                ###
;### TIME:    108 + 21*NB cycles (27.00 + 5.25*BC us)                 ###
;########################################################################
;

.globl _cpct_memset
_cpct_memset::
   ;; Recover parameters from stack
   ld   hl, #2       ;; [10] Make HL point to the byte where parameters start in the
   add  hl, sp       ;; [11] ... stack (first 2 bytes are return address)
   ld    e, (hl)     ;; [ 7] DE = Pointer to first byte in memory for memset
   inc  hl           ;; [ 6]
   ld    d, (hl)     ;; [ 7] 
   inc  hl           ;; [ 6]
   ld    c, (hl)     ;; [ 7] BC = Amount of bytes in memory to set to the value of A
   inc  hl           ;; [ 6]
   ld    b, (hl)     ;; [ 7]
   inc  hl           ;; [ 6] HL finally points to the value to be set

   ldi               ;; [16] (HL)->(DE) Copy value to the first byte of the memory to be set

   ;; Set up HL and DE for a massive copy of the Value to be set
   ld    h, d        ;; [ 4] HL = DE (2nd byte of the memory array to be filled up)
   ld    l, e        ;; [ 4] 
   dec   hl          ;; [ 6] HL-- (Point to the first position of the memory array to be
                     ;; .... filled up, which already contains the value to be set)

   ;; Copy the rest of the bytes
   ldir              ;; [21/16] Copy the reset of the bytes, cloning the first one

   ret               ;; [10] Return  
