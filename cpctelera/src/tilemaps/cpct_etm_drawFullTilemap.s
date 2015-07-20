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
.module cpct_tilemaps

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_etm_drawFullTilemap
;;
;;    Draws a complete tilemap of type TEasyTilemap
;;
;; C Definition:
;;    void <cpct_etm_drawFullTilemap> (void* *ptilemap*) __z88dk_fastcall;
;;
;; Input Parameters (2 bytes):
;;  (2B HL) ptilemap - Pointer to the TEasyTilemap structure defining the tilemap
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_etm_drawFullTilemap
;;
;; Parameter Restrictions:
;;    * *ptilemap* could be any 16-bits value, representing the memory 
;; address where the tilemap structure is stored.
;;
;; Known limitations:
;;     * This function does not do any kind of checking over the tilemap, its
;; contents or size. If you give a wrong pointer, your tilemap has different
;; dimensions than required or has less / more tiles than will be used later,
;; rubbish can appear on the screen.
;;
;; Details:
;;
;;
;; Destroyed Register values: 
;;    C Call   - AF, HL
;;    ASM Call - none
;;
;; Required memory:
;;      bytes
;;
;; Time Measures:
;; (start code)
;;    Case    | Cycles | microSecs (us)
;; ---------------------------------
;;    Any     |    
;; ---------------------------------
;; Asm saving |   
;; ---------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_etm_drawFullTilemap::
   ;; Parameter is passed in HL (Pointer to TEasyTilemap Structure)
 
   ld e, (hl)  ;; [2] DE = Pointer to the tilemap
   inc hl      ;; [2]
   ld d, (hl)  ;; [2]
   inc hl      ;; [2]
   
   ld c, (hl)  ;; [2] BC = Pointer to the tileset
   inc hl      ;; [2]
   ld b, (hl)  ;; [2]
   inc hl      ;; [2]

   ret     ;; [3] Return