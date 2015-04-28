;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_keyboard

;;
;; File: keyboard.s
;;

;; bndry directive does not work when linking previously compiled files
;.bndry 16
;;   16-byte aligned in memory to let functions use 8-bit maths for pointing
;;   (alignment not working on user linking)

;; Array: cpct_keyboardStatusBuffer
;;
;; 	10-byte *array* buffer storing keyboard status data (pressed / not pressed keys)
;; Each bit represents 1 key, with the meaning 0=pressed, 1=not pressed.
;; To know more about how this 10 bytes are distributed, consult <Keyboard>
;; and <cpct_scanKeyboard> 
;;
;;	It is used internally by all keyboard functions.
;;

cpct_keyboardStatusBuffer:: .ds 10

