;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2018 Arnaud Bouche (@Arnaud6128)
;;  Copyright (C) 2018 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_sprites

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_setReplaceColorsM0
;;
;;    Sets the colors to be used with the replace colors for Mode 0 functions
;;
;; C Definition:
;;    void <cpct_setReplaceColorsM0> (<u8> *oldColor*, <u8> *newColor*) __z88dk_fastcall;
;;
;; Input Parameters (2 bytes):
;;    (1B D) *oldColor* - Color to be replaced.
;;    (1B E) *newColor* - Color to set.
;;
;; Assembly call:
;;    > call cpct_setReplaceColorsM0
;;
;; Parameter Restrictions:
;;    * There is not check on the color value according to the Mode 0 (0 to 15)
;;
;; Destroyed Register values: 
;;      A, BC, DE
;;
;; Required memory:
;;      8 bytes
;;
;; Time Measures:
;; (start code)
;;    Case     | microSecs (us) | CPU Cycles
;; -----------------------------------------
;;     Any     |       14       |    56
;; -----------------------------------------
;; (end code)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.globl dc_mode0_ct

;; Macro to convert Pixel to xAxC xBxD format
.macro convertPixel              
    ;; From cpct_px2byteM0
    ld   bc, #dc_mode0_ct  ;; [3] BC points to conversion table (dc_mode0_ct)
    
    ;; Compute BC += A
    add  c                 ;; [1] | C += A
    ld   c, a              ;; [1] |
    sub  a                 ;; [1] A = 0 (preserving Carry Flag)
    adc  b                 ;; [1] | B += Carry
    ld   b, a              ;; [1] |

    ;; A = *(BC + A)
    ld   a, (bc)           ;; [2] A = Value stored at the table pointed by BC
.endm

.globl _cpct_color_old
.globl _cpct_color_new

   ;; Convert New color
   ld   a, h                     ;; [1]  A = H new color
   convertPixel                     ;; [10] | Convert into A
   ld   (_cpct_color_new), a     ;; [3]  | store color in memory
   
   ;; Convert Old color
   ld   a, l                     ;; [1]  A = L old color
   convertPixel                     ;; [10] | Convert into A   
   ld   (_cpct_color_old), a     ;; [3]  | store color in memory

   ret                           ;; [3]