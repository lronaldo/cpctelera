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
.module cpct_strings

;;
;; Include constants and general values
;;
.include /strings.s/

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: putchar
;;
;;    Prints a character on the screen using firmware routines. This function is 
;; required by printf when we use C standard library.
;;
;; C Definition:
;;    void <putchar> (char c)
;;
;; Details:
;;    This function is required by C standard library to be able to use printf.
;; It defines a way to print a given character to the screen. This version of the
;; function makes use of firmware to print.
;;
;; Known limitations:
;;    *Firmware* must be *ENABLED* to be able to use this function.
;; 
;; Side-effects:
;;    Calling this function with firmware disabled yields undefined behaviour, but 
;; normally leads to firmware being reenabled (unless 0xBB5A has been overwritten
;; previously).
;; 
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    14 bytes
;;
;; Time Measures:
;; (start code)
;; Case   | microSecs(us) | CPU Cycles
;; -------------------------------------
;; C Call |   16 + PC     |   64 + PC
;; -------------------------------------
;; Any    |    9 + PC     |   36 + PC
;; -------------------------------------
;; (end code)
;;    PC = Time used in the call to firmware Print Charater function
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_putchar::
_putchar_rr_s:: 
   ld   hl, #2    ;; [3] HL points to SP+2 (place where parameters start) 
   add  hl, sp    ;; [3]
   ld    a, (hl)  ;; [2] A = Character to be printed out

   call  0xBB5A   ;; [5] Firmware function for printing characters
   ret            ;; [3]

_putchar_rr_dbs::
   ld    a,e      ;; [1] A = Character to be printed out

   call  0xBB5A   ;; [5] Firmware function for printing characters
   ret            ;; [3]
