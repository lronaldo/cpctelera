;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine
;;  Copyright (C) 2017 Bouche Arnaud
;;  Copyright (C) 2017 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

;;
;; Title: Macros (asm)
;;
;;    Useful sprite-buffer related macros designed to be used in your assembler programs
;;
;; Macro: cpctm_spriteBufferPtr_asm
;;
;;    Calculates a pointer inside a sprite-buffer based on (X,Y) coordinates relative
;; to the sprite-buffer space, and WIDTH of the sprite-buffer. Calculated pointer is 
;; then loaded into a selected 16-bit register
;;
;; ASM Definition:
;;    .macro <cpctm_spriteBufferPtr_asm> *REG16*, *SPRITE*, *WIDTH*, *X*, *Y* 
;;
;; Parameters:
;;    REG16     - 16-bits register where the final result will be loaded
;;    SPRITE    - Pointer to the start of the Sprite-Buffer array
;;    WIDTH     - Width in bytes of each line of the Sprite-Buffer Array
;; care with this value not to overlap other parts of the code.
;;    X         - X coordinate of the point inside the sprite to be calculated
;;    Y         - Y coordinate of the point inside the sprite to be calculated
;; 
;; Known limitations:
;;    * This macro can only be used from assembly programs and files. It is not
;; visible from C scope. For C programs please uses <cpctm_spriteBufferPtr>.
;;    * This macro will *only* work with constant values as parameters. All the 
;; calculations must be done in compilation time. If fed with any variable parameter
;; an assembler error will be raised.
;;
.macro cpctm_spriteBufferPtr_asm REG16, SPRITE, WIDTH, X, Y 
   ld    REG16, #SPRITE + WIDTH*Y + X
.endm