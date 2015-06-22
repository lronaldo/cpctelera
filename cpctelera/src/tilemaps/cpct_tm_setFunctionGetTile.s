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
;; Function: cpct_tm_setFunctionGetTile
;;
;;    Establishes the function that will be used for getting individual tile
;; definitions (in screen pixel format) from the tileset.
;;
;; C Definition:
;;    void <cpct_tm_setFunctionGetTile> (void* pgetTileFunc);
;;
;; Input Parameters (2 bytes):
;;  (2B HL) pgetTileFunction - Pointer to the function used for getting tiles definitions from the tileset
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_tm_setFunctionGetTile_asm
;;
;; Parameter Restrictions:
;;    * *pgetTileFunction* could be any 16-bits value, representing the memory 
;; address where the code of the function starts. This function must use some 
;; concrete parameters (passed to it stored in registers) to return a pointer
;; to the start of the requested tile definition (in screen pixel format).
;;
;; Known limitations:
;;     * This function does not do any kind of checking about the given function
;; pointer. User is responsible for giving an appropriate function pointer to
;; a function that gets tile definitions from the tileset. The function must be 
;; callable from ASM, receiving parameters in HL (index of the element to get) 
;; and DE (pointer to the start of the tileset), and returning the pointer to
;; the tileset in HL. 
;;     * Not calling this function first to initialize the function pointer
;; will result in the machine being reset the first time it tries to draw
;; tiles on the screen (memory location 0x0000 will be called).
;;
;; Details:
;;    Establishes a internal pointer to a function that accesses and returns 
;; pointers to tile definitions (in screen pixel format) from the tileset. This 
;; function pointer will be used when a tile definition is required; which normally
;; happens when the tile is going to be drawn to the screen or a buffer.
;;
;;    The pointer must point to the start of the function code, to be directly
;; called from ASM code. The function will receive 2 parameters in 2 registers:
;;   (2B HL) index   - index of the tile that must be accessed
;;   (2B DE) tileset - pointer to the start of the array containing the tiles (the tileset)
;; and must return a pointer to the start of the required tile definition in HL.
;;
;;    This behaviour lets the user have different tilesets and also different
;; ways to access them. Moreover, as this is a function, special behaviours 
;; could be defined, such as returning different tiles depending on time, status
;; or other variables. That could constitute a more living tilemap / tileset, 
;; that opens up for developers imagination. 
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

_cpct_tm_setFunctionGetTile::
   ;; GET Parameters from the stack (42 cycles)
   pop  af                 ;; [10] AF = Return Address
   pop  hl                 ;; [10] HL = Pointer to the getTile Function
   push hl                 ;; [11] Left Stack as it was previously
   push af                 ;; [11]

_cpct_tm_setFunctionGetTile_asm::      ;; Assembly entry point
   ld  (#_cpct_tm_pgetTileFunc), hl    ;; [16] Store pointer internal pointer to the function 
                                       ;;      that gets tile definitions from the tileset
   ret                                 ;; [10] Return