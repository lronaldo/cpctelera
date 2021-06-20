;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
;;
;;  This program is free software: you can redistribute it and/or modify
;;  it under the terms of the GNU Lesser General Public License as published by
;;  the Free Software Foundation, either version 3 of the License, or
;;  (at your option) any later version.
;;
;;  This program is distributed in the hope that it will be useful,
;;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;  GNU Lesser General Public License for more details.
;;
;;  You should have received a copy of the GNU Lesser General Public License
;;  along with this program.  If not, see <http://www.gnu.org/licenses/>.
;;-------------------------------------------------------------------------------
;#####################################################################
;### MODULE: SetVideoMode                                          ###
;#####################################################################
;### Routines to establish and control video modes                 ###
;#####################################################################
;
.module cpct_video
   
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Function: cpct_fw2hw
;;
;;    Converts an array of firmware colour values into their equivalent hardware colour values.
;;
;; C Definition:
;;    void <cpct_fw2hw> (void* <fw_colour_array*, <u16> *size*);
;;
;; Input Parameters (4 Bytes):
;;    (2B DE) fw_colour_array - Pointer to an array of firmware colour values (in the range [0-26])
;;    (2B BC) size            - Number of colour values in the array       
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_fw2hw_asm
;;    * BC = *size* implies that B = 0, C = *size*
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
;;     C-bindings - 19 bytes
;;   ASM-bindings - 15 bytes
;;   + 27 bytes colour conversion table
;;
;; Time Measures:
;; (start code)
;;    Case    |  microSecs (us) | CPU Cycles
;; ---------------------------------------------
;;   Any      |  16 + 18*NC     |  64 + 72*NC
;; ---------------------------------------------
;; Asm saving |     -13         |    -52
;; ---------------------------------------------
;; NC= 4      |      88         |    352 
;; NC=16      |     304         |   1216 
;; (end code)
;;    NC=Number of colours to convert
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  
.globl cpct_firmware2hw_colour ;; colour conversion table defined in cpct_getHWColour.s

f2h_colour_loop:
   ld   hl, #cpct_firmware2hw_colour ;; [3] HL points to the start of the firmware2hw_colour array
   ld    a, (de)            ;; [2] A = Next colour to convert
   add   l                  ;; [1] HL += A (HL Points to the table value correspondant to A)
   ld    l, a               ;; [1] | < L += A
   adc   h                  ;; [1] | < Add Carry to H
   sub   l                  ;; [1] | |
   ld    h, a               ;; [1] | |
f2h_ncarry:
   ldi                      ;; [5] (DE) = (HL) overwrite firmware colour value with hardware colour (and HL++, DE++, BC--)
   jp   pe, f2h_colour_loop ;; [3] IF BC != 0, continue converting, else end
   
   ret                      ;; [3] Return
