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
;; Function: cpct_setBlendMode
;;
;;    Establishes the blending mode that <cpct_drawSpriteBlended> will use from
;; now on.
;;
;; C Definition:
;;    void <cpct_setBlendMode> (<CPCT_BlendMode> mode) __z88dk_fastcall;
;;
;; Input Parameters (1 bytes):
;;  (1B L) mode - New blending mode to be used for <cpct_drawSpriteBlended>
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_setBlendMode_asm
;;
;; Parameter Restrictions:
;;    * *mode* it could theoretically be any 8-bits value, but only some values
;; will be really useful, whereas some others will have unexpected results, even
;; making the program crash. In C it is restricted by the <CPCT_BlendMode> enumeration
;; to prevent errors. The value of this parameter is a Z80 opcode that will be 
;; inserted into the blending function as blend operation.
;;
;; Known limitations:
;;    * This function will produce no result if <cpct_drawSpriteBlended> is allocated 
;; in ROM space. This function modifies <cpct_drawSpriteBlended> code, therefore it
;; requires it to be in RAM space.
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

.globl _cpct_dsb_blendFunction

_cpct_setBlendMode::
cpct_setBlendMode_asm::
   ld     a, l                         ;; [1] A = Opcode
   ld    hl, #_cpct_dsb_blendFunction  ;; [3] HL points to memory place where blend opcode lies
   ld  (hl), a                         ;; [2] Set the new blend function
   ret                                 ;; [3] Return to caller
