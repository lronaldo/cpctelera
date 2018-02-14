;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine
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

;;#####################################################################
;;### MODULE: Sprites
;;### SUBMODULE: flipping.macros
;;#####################################################################
;;### Macros used to speed up calculations required for to assist
;;### flipping functions. Assembler version.
;;#####################################################################

;;
;; Macro: cpctm_spbloff
;;
;;    Macro that calculates the offset to add to a sprite pointer to point 
;; to its bottom left pixel.
;;
;; ASM Definition:
;;    .macro <cpctm_ld_sblo> *REG*, *X*, *Y*
;;
;; Acronym stands for:
;;    ld_sblo = Load Sprite Bottom Left Offset
;;
;; Parameters:
;;    (1-2B) REG - Register that will load the resulting offset (8 or 16 bits)
;;    (1B) X     - Width of the sprite in *bytes*
;;    (1B) Y     - Height of the sprite in pixels
;;
;; Parameter Restrictions:
;;    *REG* - Must be a valid 8/16 bits register that can be immediately loaded
;; using ld REG, #immediate.
;;    *X*   - Must be an immediate value representing the width of the sprite 
;; in *bytes* (Beware! Not in pixels). For sprites having interlaced mask, you 
;; may input 2 times the width of the sprite for appropriate results.
;;    *Y*   - Must be an immediate value representing the height of the sprite 
;; in pixels.
;;
;; Returns:
;;    REG = X * (Y - 1) ;; Register loaded with the offset
;;
;; Details:
;;    This macro calculates the offset of the initial byte of the last row 
;; of a given sprite (i.e. its bottom-left byte), with respect to its first
;; byte (top-left corner). This value can easily be added to any sprite 
;; pointer to get a pointer to the bottom-left byte. This pointer is required
;; byte many flipping functions (like <cpct_vflipSpriteM0>). Values for width
;; and height of the sprite must be constant immediate values. Otherwise, this
;; macro will generate incorrect code that will fail to compile. 
;;    The macro calculates *X* * (*Y*-1) at compile-time and loads it into
;; the given register. Please, take into account that the macro does no check
;; about the size of the resulting values. If multiplication results in a value
;; greater than 255, you will need to load it into a 16-bit register. You must
;; take care of the expected size of the offset value.
;;
;; Known issues:
;;    * This is a assembler macro. It cannot be called or used from C code.
;;
.macro cpctm_ld_sblo REG, X, Y
   ld    REG, #X * (Y-1)
.endm