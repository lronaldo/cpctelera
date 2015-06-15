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
;; Function: cpct_setGetTileFuncion
;;
;;    Estabilishes the function that will be used for getting individual tile
;; indices from a tilemap array. 
;;
;; C Definition:
;;    void <cpct_setGetTileFunction> (void* pgetTileFunction);
;;
;; Input Parameters (2 bytes):
;;  (2B HL) pgetTileFunction - Pointer to the function used for getting tiles_idxs from tilemap
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_setGetTileFunction_asm
;;
;; Parameter Restrictions:
;;    * *pgetTileFunction* could be any 16-bits value, representing the memory 
;; address where the code for an array access function starts.
;;
;; Known limitations:
;;     * This function does not do any kind of checking about the given function
;; pointer. User is responsible for giving an appropriate function pointer to
;; a function that gets elements from an array. The function must be callable 
;; from ASM, receiving parameters in HL (index of the element to get) and 
;; DE (pointer to the start of the array), and returning the vale of the element got
;; in L or HL (the value of the required index). 
;;     * Not calling this function first to initialize the function pointer
;; will result in the machine being resetted the first time it tries to draw
;; tiles on the screen (memory location 0x0000 will be called).
;;
;; Details:
;;    Estabilishes a internal pointer to a function that accesses and returns 
;; elements from an array. This function pointer will be used when the tilemaps
;; module accesses the elements of the tilemap (the individual tile ids) to 
;; draw them on the screen.
;;
;;    The pointer must point to the start of the function code, to be directly
;; called from ASM code. The function will receive 2 parameters in 2 registers:
;;   (2B HL) index - index of the element that must be accessed
;;   (2B DE) array - pointer to the start of the array where the element is
;; and must return the value of the required index in L or HL registers (depending
;; on the returned value being 8-bits or 16-bits long).
;;
;;    This behaviour lets the user have different tilemaps with the required
;; proportion of bits per tile index. For instance, when managing a tileset of
;; 16 different tiles, only 4-bits are required for each tile index in the tilemap;
;; therefore, <cpct_get4Bits_asm> function could be used to have a tilemap that
;; only uses 4-bits for each tile index.
;;
;;    It is *very important* to call this function previously to the use of 
;; tilemap managing functions; otherwise, computer will reset itself when 
;; tilemanaging function try to draw tiles on the screen, as 0x0000 memory
;; location will be called.
;;
;; Destroyed Register values: 
;;    C Call   - AF, HL
;;    ASM Call - none
;;
;; Required memory:
;;     8 bytes
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

_cpct_setGetTileFunction::
   ;; GET Parameters from the stack (42 cycles)
   pop  af                 ;; [10] AF = Return Address
   pop  hl                 ;; [10] HL = Pointer to the getTileFromArray Function
   push hl                 ;; [11] Left Stack as it was previously
   push af                 ;; [11]

_cpct_setGetTileFunction_asm::        ;; Assembly entry point
   ld  (#_cpct_pgetTileFromArray), hl ;; [16] Store pointer to the tileset in our internal variable
   ret                                ;; [10] Return