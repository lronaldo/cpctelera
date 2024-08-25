;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2021 Joaquín Ferrero (https://github.com/joaquinferrero)
;;  Copyright (C) 2021 Nestornillo (https://github.com/nestornillo)
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
.module cpct_divideby10
  
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Function: cpct_divideby10
;;
;;    Returns a byte that is the result of dividing an integer number by 10. 
;;
;; C Definition:
;;    <u8> <cpct_divideby10> (u8 dividendo)
;;
;; Input Parameters (4 Bytes):
;;    (1B A ) dividendo  - [0-255]  Byte
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_divideby10_asm
;;
;; Parameter Restrictions:
;;    * Parameter is among 0 and 255. 
;; Return Value:
;;    (A,L) u8 - Result of the operation.
;;
;; Known limitations:
;;
;; Details:
;; This function returns the result of the division of a given number divided
;; by 10 (decimal). 
;; N/10 = N/2 * 1/5 ,is near to N/2 * 13/64
;; 13/64 = (1 + 4 + 8) / 64 = (1/8 + 1/2 + 1) / 8
;; So, N/10 aproximately is (N/16 + N/4 + N/2) / 8
;;
;; Destroyed Register values:
;;    AF, BC,HL
;;
;; Required memory:
;;    C-bindings - 22 bytes
;;  ASM-bindings - 19 bytes
;;
;; Time Measures: 
;; (start code)
;;     Case   | microSecs (us) | CPU Cycles
;; -----------------------------------------
;;     Any    |      43        |     172
;; -----------------------------------------
;; Asm saving |     -13        |     -52
;; -----------------------------------------
;; (end code)
;;
;;
ld bc,#0xC0FE ; [3] Take a coffee to start a good day:)
srl a     ; [2]  A = N/2
add b     ; [1]  Adjust input values 
sbc b     ; [1]    greater than 127
ld b,a    ; [1]  B = N/2
and c     ; [1]
;; Divide A=N to get the final result  
;; A = (N/16 + N/4 + N/2) / 8
;;divide N by 8 A=(N/8)
rra       ; [1]  A = N/4
rra       ; [1]  A = N/8
;;add N/2
add b     ; [1]  A = N/8  + N/2
;;divide by 2 A=N/8 + N/2)*2
rra       ; [1]  A = N/16 + N/4
;;add N/2
add b     ; [1]  A = N/16 + N/4 + N/2
;;divide by 2 A=(N/16 + N/4 + N/2) / 2
rra       ; [1]  A = (N/16 + N/4 + N/2) / 2
and c     ; [1]
;;final result A = (N/16 + N/4 + N/2) / 8
rra       ; [1]  A = (N/16 + N/4 + N/2) / 4
rra       ; [1]  A = (N/16 + N/4 + N/2) / 8
;;return result in L
ld l,a    ; 
;;ret       ; [3]