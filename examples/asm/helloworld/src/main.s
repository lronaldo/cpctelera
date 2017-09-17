;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;; Start of _DATA area
;;    (SDCC requires at least _DATA and _CODE areas to be declared. )
;;
.area _DATA

;; Define a welcome string message
string: .asciz "Welcome to CPCtelera in ASM!"

;;
;; Start of _CODE area
;; 
.area _CODE

;; Symbols with the names of the CPCtelera functions we want to use
;; must be declared globl to be recognized by the compiler. Later on,
;; linker will do its job and make the calls go to function code.
.globl cpct_disableFirmware_asm
.globl cpct_drawStringM1_asm

;;
;; MAIN function. This is the entry point of the application.
;;    _main:: global symbol is required for correctly compiling and linking
;;
_main::

   ;; Disable firmware to prevent it from interfering with drawString
   call cpct_disableFirmware_asm

   ;; We are going to call draw String, and we have to push parameters
   ;; to the stack first (as the function recovers it from there).
   ld   hl, #string  ;; HL = Pointer to the start of the string
   ld   de, #0xC280  ;; DE = Pointer to video memory location where the string will be drawn
   ld   bc, #0x0003  ;; B = Background colour, C = Foreground colour

   call cpct_drawStringM1_asm ;; Call the string drawing function

forever:
   jp forever        ;; Infinite waiting loop
