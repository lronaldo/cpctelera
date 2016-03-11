;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_setDrawSpriteBlendFunction
;;
;; TODO (xor by default)
;;
;; C Definition:
;;    void <cpct_setDrawSpriteBlendFunction> (u8 function) __z88dk_fastcall;
;;
;; Input Parameters (6 bytes):
;;  (1B L) function - Opcode of the function to be applied to blend bytes
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_setDrawSpriteBlendFunction
;;
;; Parameter Restrictions:
;;
;; TODO: REVIEW
;;
;; Known limitations:
;;
;; TODO: REVIEW
;;
;; Details:
;;
;; TODO: REVIEW
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    C-bindings - xx bytes 
;;  ASM-bindints - xx bytes
;;
;; Time Measures:
;; (start code)
;;  Case      | microSecs (us) | CPU Cycles
;; -------------------------------------------
;;  Any       |       9        |     36                        
;; -------------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_setDrawSpriteBlendFunction::
cpct_setDrawSpriteBlendFunction_asm::
   ld     a, l                         ;; [1] A = Opcode
   ld    hl, #_cpct_dsb_blendFunction  ;; [3] HL points to memory place where blend opcode lies
   ld  (hl), a                         ;; [2] Set the new blend function
   ret                                 ;; [3] Return to caller
