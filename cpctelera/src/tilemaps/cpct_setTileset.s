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

.include /tilemaps.s.inc/

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_setTileset
;;
;;    Estabilishes the set of tiles that will be used by tilemap managing 
;; functions.
;;
;; C Definition:
;;    void <cpct_setTileset> (void* tileset);
;;
;; Input Parameters (7 bytes):
;;  (2B HL) ptileset - Pointer to the tileset 
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_setTileset
;;
;; Parameter Restrictions:
;;    * *ptileset* could be any 16-bits value, representing the memory 
;; address where the tileset is stored (first byte).
;;
;; Known limitations:
;;     * This function does not do any kind of checking over the tilset, its
;; contents or size. If you give a wrong pointer, your tileset has different
;; dimensions than required or has less/more tiles than will be used later,
;; rubbish can appear on the screen.
;;
;; Details:
;;    Estabilishes a internal pointer that is used by tiledrawing functions
;; to read tiles from the tileset. This pointer should point to the first 
;; byte in memory where the tileset is stored. 
;;
;;    It is *very important* to call this function previously to the use of 
;; tilemap managing functions; otherwise, functions will access random
;; bytes of memory as tile definitions, which will lead to drawing problems.
;;
;; Destroyed Register values: 
;;     AF, HL
;;
;; Required memory:
;;     bytes
;;
;; Time Measures:
;; (start code)
;;    Case    | Cycles | microSecs (us)
;; ---------------------------------
;;    Any     |    68  |   17.00
;; ---------------------------------
;; Asm saving |   -42  |  -10.50
;; ---------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_setTileset::
   ;; GET Parameters from the stack (42 cycles)
   pop  af                 ;; [10] AF = Return Address
   pop  hl                 ;; [10] HL = Pointer to the tileset
   push hl                 ;; [11] Left Stack as it was previously
   push af                 ;; [11]

_cpct_setTileset_asm::       ;; Assembly entry point
   ld  (#_cpct_ptileset), hl ;; [16] Store pointer to the tileset in our internal variable
   ret                       ;; [10] Return