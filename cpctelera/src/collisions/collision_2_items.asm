;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine
;;
;;  Copyright (C) 2024 raulgarfer (https://github.com/raulgarfer)
;;  Copyright (C) 2024 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module collisions
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_detect_collision
;;
;; Returns if there was a collision betwwen two items.
;;
;; C Definition:
;;    <u8> <cpct_detect_collision> (u8* items) __z88dk_fastcall;
;;
;; Input Parameters (1 Byte):
;;    (2B HL) Array - Start of the data in use
;;
;; Assembly call (Input parameter in HL register):
;;    > call cpct_divideby10_asm
;;
;; Return Value (Assembly call, returned in A register):
;;    <u8> - Result of the collision check.
;;
;; Parameter Restrictions:
;;    * No data validity check is performed.
;;
;; Description:
;;   This function checks if a collision has occurred between two known entities.
;; It does so by calculating the boundaries of each entity and comparing them with the other.
;;
;; A--ENT1--B  C--ENT2--D
;;
;;    For the first check, the X position of entity 1 (A) is added to its width (B).
;; Then, the X position of the second entity (C) is subtracted from this result. If the result is positive 
;; (C<B, no carry), the next step is calculated. If the result is negative (C>B, carry occurs), the function
;; ends, returning 0 in A. 
;;
;; C--ENT2--D  A--ENT1--B  
;;    The next checks are D against A, on the X axis. After this, the Y coordinates are checked in the same way.
;;    If all four checks indicate a collision, 1 is returned in A.
;;
;; Destroyed Register values:
;;    AF, HL
;;
;; Required memory:
;;    C-bindings - 35 bytes
;;  ASM-bindings - 35 bytes
;;
;; Time Measures:
;; (start code)
;;     Case   | microSecs (µs) | CPU Cycles
;; -----------------------------------------
;;     Best   |      18        |     72
;; -----------------------------------------
;;     Worst  |      51        |     204
;; -----------------------------------------
;; Asm saving |      -2        |     -8
;; -----------------------------------------
;; (end code)
;;
;; Credits:
;;    Adapted code by Fran Gallego. 
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Input HL as an array to check. The order should be as follows:
;; HL+0 = X position of entity 1
;; HL+1 = Width of entity 1
;; HL+2 = X position of entity 2
;; HL+3 = Width of entity 2
;; HL+4 = Y position of entity 1
;; HL+5 = Height of entity 1
;; HL+6 = Y position of entity 2
;; HL+7 = Height of entity 2
;;
;; First, load the X position of entity 1 and add its width
        ld a,(hl)       ;; A = X position of entity 1
        ld b,a          ;; B = X position of entity 1
        inc hl          ;; 
        add a,(hl)      ;; A += Width of entity 1
        inc hl          ;;
        sub a,(hl)      ;; A -= X position of entity 2
            jr c,no_collision 
;; Now check the right side of entity 2 against the X position of entity 1  
        ld a,(hl)         ;; A = X position of entity 2 
        inc hl            ;;  
        add a,(hl)        ;; A += Width of entity 2
        sub b             ;; A -= X position of entity 1
           jr c,no_collision
;; The two entities overlap on the X axis.  
;; Now check the bottom of entity 1 against the Y position of entity 2
        inc hl            ;;
        ld a,(hl)         ;; A = Y position of entity 1
        ld b,a            ;; B = Y position of entity 1
        inc hl            ;;  
        add a,(hl)        ;; A += Height of entity 1
        inc hl            ;;
        sub a,(hl)        ;; A -= Y position of entity 2
            jr c,no_collision ;;
;; Now check the bottom of entity 2 against the Y position of entity 1  
        ld a,(hl)         ;; A = Y position of entity 2 
        inc hl            ;;  
        add a,(hl)        ;; A += Height of entity 2
        sub b             ;; Restore Y position of entity 1 and subtract
            jr c,no_collision ;;
;; Both entities overlap on both the X and Y axes. A collision has occurred.
;; Return a non-zero value in register A to indicate a collision
        ld a,#1           ;; 
ret
        
no_collision:
;; After confirming no collision, return 0 to indicate no collision occurred
        ld a,#0           ;;


