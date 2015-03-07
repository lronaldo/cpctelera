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
;#####################################################################
;### MODULE: SetVideoMode                                          ###
;#####################################################################
;### Routines to establish and control video modes                 ###
;#####################################################################
;
.module cpct_videomode

.include /videomode.s/

;
;########################################################################
;## FUNCTION: _cpct_setVideoPalette                                   ###
;########################################################################
;### This function modifies video palette registers to set the INKs   ###
;### colours to use. Palette registers are contained inside the PAL   ###
;### chip of the CPC (which is located inside the Gate Array). To     ###
;### access PAL chip, the same port (&7F) as Gate Array is used.      ###
;### The function is not thought to be optimal, but secure. If a more ###
;### optimal one is required, it can be derived from this code.       ###
;###                                                                  ###
;### Known limitations: this function cannot set blinking colors, as  ###
;### blinking colours are managed by the firmware (not by hardware)   ###
;########################################################################
;### INPUTS (3 Bytes)                                                 ###
;###  * (2B DE) Pointer to an array of bytes containing colour numbers### 
;###  * (1B A) Number of colours to set (up to 16)                    ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURED TIME                                                    ###
;### 101 + 78*C (C = number of colours to be set, 2nd parameter)      ###
;### Example:                                                         ###
;###  16 colours = 1349 cycles (337.25 ns)                            ###
;########################################################################
;### CREDITS:                                                         ###
;###   This function has been constructed with great help from the    ###
;### documentation at Grimware about the Gate Array.                  ###
;### http://www.grimware.org/doku.php/documentations/devices/gatearray###
;########################################################################
;
_cpct_setVideoPalette::
   ;; Getting parameters from stack
   LD  HL, #2               ;; [10] HL = SP + 2 (Place where parameters start) 
   ADD HL, SP               ;; [11]
   LD   E, (HL)             ;; [ 7] DE = First parameter, pointer to the array that contains desired color IDs 
   INC  HL                  ;; [ 6]      (should get it in little endian order, E, then D)
   LD   D, (HL)             ;; [ 7]
   INC  HL                  ;; [ 6]
   LD   A, (HL)             ;; [ 7] A = Second parameter, Number of colours to set (up to 16) 

   EX   DE, HL              ;; [ 4] HL = DE, We will use HL to point to the array and get colour ID values
   DEC  A                   ;; [ 4] A -= 1 (We leave A as 1 colour less to convert 16 into 15 and be able to use AND to ensure no more than 16 colours are passed)
   AND  #0x0F               ;; [ 7] A %= 16, A will be 15 at most, that is, the number of colours to set minus 1
   INC  A                   ;; [ 4] A += 1 Restore the 1 to leave A as the number of colours to set
   LD   E, A                ;; [ 4] E = A, E will have the number of colours to set 
   LD   D, #0               ;; [ 7] D = 0, D will count from 0 to E 

   LD   B, #GA_port_byte    ;; [ 7] BC = Gate Array Port (0x7F), to send commands using OUT
svp_setPens_loop:
   LD   A, D                ;; [ 4] A = D, A has index of the next PEN to be established
  ;OR   #PAL_PENR           ;; [ 7] A = (CCCnnnnn) Mix 3 bits for PENR command (C) and 5 for PEN number (n). As PENR command is 000, nothing to be done here.
   OUT (C),A                ;; [11] GA Command: Select PENR. A = Command + Parameter (PENR + PEN number to select)

   LD   A, (HL)             ;; [ 7] Get Color value (INK) for the selected PEN from array
   AND  #0x1F               ;; [ 7] Leave out only the 5 Least significant bits (3 MSB can cause problems)
   OR   #PAL_INKR           ;; [ 7] (CCCnnnnn) Mix 3 bits for INKR command (C) and 5 for PEN number (n). 
   OUT (C),A                ;; [11] GA Command: Set INKR. A = Command + Parameter (INKR + INK to be set for selected PEN number)

   INC  HL                  ;; [ 6] HL += 1, Point to the next INK in the array
   INC  D                   ;; [ 4] D += 1, Next PEN index to be set 
   DEC  E                   ;; [ 4] E -= 1, count how many PENs still to be set
   JP NZ, svp_setPens_loop  ;; [10] If more than 0 PENs to be set, continue

   RET                      ;; [10] Return
