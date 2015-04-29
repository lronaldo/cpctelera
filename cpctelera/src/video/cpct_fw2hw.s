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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Function: cpct_fw2hw
;;
;;    Converts an array of firmware colour values into their equivalent hardware colour values.
;;
;; C Definition:
;;    void <cpct_fw2hw> (void* <fw_colour_array*, <u8> *size*);
;;
;; Input Parameters (3 Bytes):
;;    (2B DE) fw_colour_array - Pointer to an array of firmware colour values (in the range [0-26])
;;    (1B A)  size            - Number of colour values in the array       
;;
;; Parameter Restrictions:
;;    * *fw_colour_array* must be an array of values in the range [0-26], otherwise, return 
;; value will be unexpected.
;;    * *size* must be the number of elements in the array, and must be at least 1. A size of 0 
;; would make this function overwrite 256 values in memory. Similarly, a value greater than the
;; actual size of the array would result in some values outside the array being overwritten.
;;
;; Details:
;;    Converts an array of firmware colour values into their equivalent hardware colour values. 
;; It directly modifies the array passed to the function, overwritting the its values with 
;; the hardware colour values.
;;
;; Destroyed Register values:
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    59 bytes (32 bytes code, 27 bytes colour conversion table)
;;
;; Time Measures:
;; (start code)
;; Case  |   Cycles    |  microSecs (us)
;; ---------------------------------------
;; Best  |  68 + 61*NC |  17.00 + 15.25*NC
;; Worst |  68 + 65*NC |  17.00 + 16.25*NC
;; ---------------------------------------
;; NC= 8 |  556 /  588 |  139.00 / 147.00
;; NC=16 | 1044 / 1108 |  261.00 / 277.00 
;; (end code)
;;    NC=Number of colours to convert
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   
_cpct_fw2hw::
   ld   hl, #2              ;; [10] HL = SP + 2 (Place where parameters start) 
   ld    b, h               ;; [ 4] B = 0, (BC = C) so that we can use BC as counter
   add  hl, sp              ;; [11]
   ld    e, (hl)            ;; [ 7] DE = Pointer to colour array
   inc  hl                  ;; [ 6]
   ld    d, (hl)            ;; [ 7]
   inc  hl                  ;; [ 6]
   ld    c, (hl)            ;; [ 7] C = Number of colours to convert 

f2h_colour_loop:
   ld   hl, #cpct_firmware2hw_colour ;; [10] HL points to the start of the firmware2hw_colour array
   ld    a, (de)            ;; [ 7] A = Next colour to convert
   add   l                  ;; [ 4] HL += A (HL Points to the table value correspondant to A)
   ld    l, a               ;; [ 4] |  
   jp   nc, f2h_ncarry      ;; [10] |   (8-bits sum: only when L+A generates carry, H must be incremented)
   inc   h                  ;; [ 4] \
f2h_ncarry:
   ldi                      ;; [16] (DE) = (HL) overwrite firmware colour value with hardware colour (and HL++, DE++)
   jp   pe, f2h_colour_loop ;; [10] IF BC != 0, continue converting, else end
   
   ret                      ;; [10] Return
