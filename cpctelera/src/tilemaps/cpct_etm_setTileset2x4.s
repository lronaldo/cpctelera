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
;; Function: cpct_etm_setTileset2x4
;;
;;    Sets the tileset that will be used by EasyTilemap's tiledrawing functions.
;;
;; C Definition:
;;    void <cpct_etm_setTileset> (const void* *ptileset*) __z88dk_fastcall;
;;
;; Input Parameters (5 bytes):
;;    (2B HL) ptileset - Pointer to the start of the tileset (array of pointers to tile definitions)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_etm_drawTileRow_asm
;;
;; Parameter Restrictions:
;;    * *ptileset* could be any 16-bits value, representing the memory address where 
;; the tileset is stored. This function expects the parameter to point to an array of
;; pointers to tile definitions, but does not performe any kind of check. If you provide
;; a pointer to a different kind of data, the result will be undefined behaviour (
;; tipically nothing or rubbish will be drawn when tilemap drawing functions get called)
;;
;; Important details:
;;    * This function *MUST* be called at least once previous to the use of other
;; EasyTilemap's drawing functions (<cpct_etm_drawFullTileMap>, <cpct_etm_drawTileRow>).
;; Otherwise, those functions will use 0000 as pointer to the tileset, which will lead
;; to undefined behaviour (tipically nothing or rubbish will be drawn on screen).
;;
;; Details:
;;    This function sets the default tileset that will be used by Easytilemap's tiledrawing
;; functions. The function inserts the pointer value into the required functions's code
;; to make that functions load the pointer value by default and use it.
;;
;; Destroyed Register values: 
;;      none
;;
;; Required memory:
;;      2 bytes
;;
;; Time Measures:
;; (start code)
;;    Case     | microSecs (us) | CPU Cycles
;; -----------------------------------------
;;    Any      |      9         |    36
;; -----------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; This is a label inside cpct_drawTileRow that points to the placeholder where
;;   tileset address is to be configured
.globl dtr_restore_ptileset

_cpct_etm_setTileset2x4::
cpct_etm_setTileset2x4_asm::
   ;; No need to recover parameters from the stack, as this function uses __z88dk_fastcall
   ;; convention, which means parameter is passed directly in HL

   ld  (dtr_restore_ptileset + 1), hl   ;; [6]
   ret                                  ;; [3] Return
