;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2019 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_strings

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: getchar
;;
;;    Reads a character from the input keyboard buffer and returns its ASCII code. 
;; If no character is available, it blocks until one is inserted. 
;;
;; C Definition:
;;    char <getchar> ()
;;
;; Details:
;;    This function may be required by other C standard library functions such as
;; scanf or gets. It defines a way to get a single input character read from 
;; keyboard, in ASCII form. This version of the function makes use of firmware 
;; to read from keyboard.
;;    This function is designed to be used from a C language call only. Although 
;; it could be called from assembly code, it is not advised. You may prefer to
;; call 0xBB06 directly instead and save these bytes and cycles.
;;
;; Known limitations:
;;    *Firmware* must be *ENABLED* to be able to use this function.
;; 
;; Side-effects:
;;    Calling this function with firmware disabled yields undefined behaviour, but 
;; normally leads to firmware being reenabled (unless 0xBB06 has been overwritten
;; previously).
;; 
;; Destroyed Register values: 
;;    AF, L
;;
;; Required memory:
;;     5 bytes
;;
;; Time Measures:
;; (start code)
;; Case   | microSecs(us) | CPU Cycles
;; -------------------------------------
;; C Call |    9 + WC     |   36 + WC
;; -------------------------------------
;; (end code)
;;    WC = Time used in the call to firmware KM_WAIT_CHAR function
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_getchar::
   call  0xBB06   ;; [5] Firmware function to wait for a character input from keyboard
   ld    l, a     ;; [1] L=A (ASCII code of the keyboard input to be returned)
   ret            ;; [3]