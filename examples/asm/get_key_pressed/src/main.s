;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2018 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
;;  Copyright (C) 2024 Raul Garcia (@kalandras)
;;  Credits to Nestor (@Nestornillo) for his help improving the code
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
;; Include all CPCtelera constant definitions, macros and variables
.include "cpctelera.h.s"
;;
;; Start of _DATA area 
;;  SDCC requires at least _DATA and _CODE areas to be declared, but you may use
;;  any one of them for any purpose. Usually, compiler puts _DATA area contents
;;  right after _CODE area contents.
;;
.area _DATA
;;
;; Start of _CODE area
;; 
.area _CODE
;; 
;; Declare all function entry points as global symbols for the compiler.
;; (The linker will know what to do with them)
;; WARNING: Every global symbol declared will be linked, so DO NOT declare 
;; symbols for functions you do not use.
;;
.globl cpct_setDrawCharM1_asm
.globl cpct_getKeypressedAsASCII_asm
.globl cpct_drawCharM1_asm
.globl cpct_scanKeyboard_asm
;;
;; MAIN function. This is the entry point of the application.
;;    _main:: global symbol is required for correctly compiling and linking
;;
_main::
;; Set up draw char colours before calling draw string
   ld    d, #0         ;; D = Background PEN (0)
   ld    e, #3         ;; E = Foreground PEN (3)
   call cpct_setDrawCharM1_asm   ;; Set draw char colours
loop:
;; Reads the status of keyboard and joysticks.
   call cpct_scanKeyboard_asm
;;
;;  Assuming there is only one Key currently pressed, it returns the ASCII
;; value associated to the pressed key. 
;; Return: Register A as ASCII code of the key pressed (if any)
   call cpct_getKeypressedAsASCII_asm
;;
;; Prints ASCII character in the position given by HL.
;; Input Parameters (3 Bytes)
;; (2B HL) video_memory	Video memory location where the character will be drawn
;; (1B E ) ascii	Character to be drawn (ASCII code)
;; Note: Only ASCII capitalized letters will be printed.
   ld hl,#0xc000        ;;Start of Video Memory
   ld e,a               ;;Load input parameter
   call cpct_drawCharM1_asm
;;   
;; Loop forever
   jr    loop