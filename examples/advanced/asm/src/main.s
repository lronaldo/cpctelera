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
.globl _cpct_disableFirmware
.globl _cpct_drawStringM1

;;
;; MAIN function. This is the entry point of the application.
;;    _main:: global symbol is required for correctly compiling and linking
;;
_main::

   ;; Disable firmware to prevent it from interfering with drawString
   call _cpct_disableFirmware

   ;; We are going to call draw String, and we have to push parameters
   ;; to the stack first (as the function recovers it from there).
   ld   hl, #string  ;; HL = Pointer to the start of the string
   ld   de, #0xC280  ;; DE = Pointer to video memory location where the string will be drawn
   ld   bc, #0x0003  ;; B = Background colour, C = Foreground colour

   push bc           ;; Push parameters to the stack in reverse order. To enable the function
   push de           ;; to get them in normal order, as the stack is LIFO (Last in, First Out).
   push hl           
   call _cpct_drawStringM1 ;; Call the string drawing function
   ld   hl,#6        ;; Restore stack status. As we have pushed 6 bytes to the stack, we 
   add  hl,sp        ;; have to move Stack Pointer (SP) 6 bytes forward to leave it at its
   ld   sp,hl        ;; previous location. 

forever:
   jp forever        ;; Infinite waiting loop
