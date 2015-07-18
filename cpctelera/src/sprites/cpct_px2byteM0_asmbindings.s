;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

;; Pixel colour table defined in cpct_drawCharM0
.globl dc_mode0_ct

;;
;; Macro that computes A = *(DE + A)
;;  to access vales stored in tables pointed by DE
;;
.macro A_eq__DEplusA__
   ;; Compute DE += A
   add   e               ;; [1] | E += A
   ld    e, a            ;; [1] |
   sub   a               ;; [1] A = 0 (preserving Carry Flag)
   adc   d               ;; [1] | D += Carry
   ld    d, a            ;; [1] |

   ;; A = *(DE + A)
   ld    a, (de)         ;; [2] A = Value stored at the table pointed by DE 
.endm

;;
;; ASM-bindings for the function <cpct_px2byteM0>
;;   As this function is so particular, this is a complete version of the function
;; for assembly calls, that works different from the C version.
;;   HL = Pixel 0, Pixel 1 vales
;;  
cpct_px2byteM0_asm::
   ;;
   ;; Transform pixel 0 into Screen Pixel Format
   ;;
   ld   de, #dc_mode0_ct ;; [3] DE points to the start of the colour table
   ld    a, h            ;; [1] A = Firmware colour for pixel 0 (to be added to DE, 
                         ;; .... as it is the index of the colour value to retrieve)
   
   ;; Get screen format bit values for Pixel 0
   A_eq__DEplusA__       ;; [7] A = *(DE + A)

   sla   a               ;; [2] A <<= 1, as Screen formats in table are in Pixel Y disposition 
                         ;; .... (see Scheme 1 in this function's documentation)
   ld    b, a            ;; [1] B = Transformed value for pixel 0

   ;;
   ;; Transform pixel 1 into Screen Pixel Format
   ;;
   ld   de, #dc_mode0_ct ;; [3] DE points to the start of the colour table
   ld    a, l            ;; [1] A = Firmware colour for pixel 1 (to be added to DE, 
                         ;; .... as it is the index of the colour value to retrieve)

   ;; Get screen format bit values for Pixel 1
   A_eq__DEplusA__       ;; [7] A = *(DE + A)
   
   ;; Merge both values and return result
   or    b               ;; [1] A = Merged value of transformed pixel values (px1 | px2)
   ld    l, a            ;; [1] L = A, put return value into L

   ret                   ;; [3] return
