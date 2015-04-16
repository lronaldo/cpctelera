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
;## FUNCTION: _cpct_fw2hw                                             ###
;########################################################################
;### Converts an array of firmware colour values into their equivalent###
;### hardware colour values. It directly modifies the array passed to ###
;### the function.                                                    ###
;########################################################################
;### INPUTS (3 Bytes)                                                 ###
;###  * (2B DE) Array of firmware colour values (0-26 each)           ###
;###  * (1B A)  Number of colour values in the array                  ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURES                                                         ###
;###  MEMORY: 59 bytes (32 code + 27 colour table)                    ###
;###  TIME: 68 + 65*NumColours cycles (17.00 + 16.25*NC us)           ###
;### Example:                                                         ###
;###  16 colours, 1108 cycles (277,00 us)                             ###
;########################################################################
;
   
_cpct_fw2hw::
   LD  HL, #2               ;; [10] HL = SP + 2 (Place where parameters start) 
   LD   B, H                ;; [ 4] B = 0, (BC = C) so that we can use BC as counter
   ADD HL, SP               ;; [11]
   LD   E, (HL)             ;; [ 7] DE = Pointer to colour array
   INC HL                   ;; [ 6]
   LD   D, (HL)             ;; [ 7]
   INC HL                   ;; [ 6]
   LD   C, (HL)             ;; [ 7] C = Number of colours to convert 

f2h_colour_loop:
   LD  HL, #cpct_firmware2hw_colour ;; [10] HL points to the start of the firmware2hw_colour array
   LD   A, (DE)             ;; [ 7] A = Next colour to convert
   ADD  L                   ;; [ 4] HL += C (HL Points to the table value correspondant to A)
   LD   L, A                ;; [ 4] |  
   JP  NC, f2h_ncarry       ;; [10] |   (8-bits sum: only when L+C generates carry, H must be incremented)
   INC  H                   ;; [ 4] \
f2h_ncarry:
   LDI                      ;; [16] (DE) = (HL) overwrite firmware colour value with hardware colour (and HL++, DE++)
   JP  PE, f2h_colour_loop  ;; [10] IF BC != 0, continue converting, else end
   
   RET                      ;; [10] Return
