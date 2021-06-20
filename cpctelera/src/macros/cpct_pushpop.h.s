;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2020 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;; File: Push - Pop Macros
;;
;;    Useful macros to simplify push-pop save/restore operations
;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Macro: cpctm_push RO, R1, R2, R3, R4, R5
;;
;;    Pushes any given registers (up to 6) into the stack
;;
;; ASM Definition:
;;    .macro <cpctm_push> R0, R1, R2, R3, R4, R5
;;
;; Parameters:
;;    R0-R5 - Any number of 16-bit pushable registers, up to 6
;;
;; Details:
;;    This macro converts the list of 16-bit registers given as parameters into a list
;; of 'push' operations to push all of them into the stack. The registers are pushed
;; into the stack in the same order as they are given in the parameter list.
;;    The macro accepts any number of registers up to the maximum of 6 that are 
;; predefined as parameters. However, you may use it with 1, 2, 3, 4 or 5 registers
;; as parameters. There is no need to give the 6 parameters: only those given will 
;; be considered.
;;
;; Modified Registers: 
;;    none
;;
;; Required memory:
;;    1 byte per register given (2 if they are IX or IY)
;;
;; Time Measures:
;; (start code)
;;  Case     | microSecs(us) | CPU Cycles
;; ------------------------------------
;;  Per Reg  |       4       |     16
;; ------------------------------------
;;  Per IX/IY|       5       |     20
;; ------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.macro cpctm_push r0, r1, r2, r3, r4, r5
   .narg v
   .if v
   push r0
   .if v-1
   push r1
   .if v-2
   push r2
   .if v-3
   push r3
   .if v-4
   push r4
   .if v-5
   push r5
   .else
   .mexit
   .endif
   .else
   .mexit
   .endif
   .else
   .mexit
   .endif
   .else
   .mexit
   .endif
   .else
   .mexit
   .endif
   .else
   .mexit
   .endif
.endm


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Macro: cpctm_pop RO, R1, R2, R3, R4, R5
;;
;;    Pops any given registers (up to 6) from the stack
;;
;; ASM Definition:
;;    .macro <cpctm_pop> R0, R1, R2, R3, R4, R5
;;
;; Parameters:
;;    R0-R5 - Any number of 16-bit pushable/popable registers, up to 6
;;
;; Details:
;;    This macro converts the list of 16-bit registers given as parameters into a list
;; of 'pop' operations to pop all of them from the stack. The registers are poped
;; in the same order as they are given in the parameter list.
;;    The macro accepts any number of registers up to the maximum of 6 that are 
;; predefined as parameters. However, you may use it with 1, 2, 3, 4 or 5 registers
;; as parameters. There is no need to give the 6 parameters: only those given will 
;; be considered.
;;
;; Modified Registers: 
;;    R0, R1, R2, R3, R4, R5 (Those given as parameters are loaded from the stack)
;;
;; Required memory:
;;    1 byte per register given (2 if they are IX or IY)
;;
;; Time Measures:
;; (start code)
;;  Case     | microSecs(us) | CPU Cycles
;; ------------------------------------
;;  Per Reg  |       3       |     12
;; ------------------------------------
;;  Per IX/IY|       5       |     20
;; ------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.macro cpctm_pop r0, r1, r2, r3, r4, r5
   .narg v
   .if v
   pop r0
   .if v-1
   pop r1
   .if v-2
   pop r2
   .if v-3
   pop r3
   .if v-4
   pop r4
   .if v-5
   pop r5
   .else
   .mexit
   .endif
   .else
   .mexit
   .endif
   .else
   .mexit
   .endif
   .else
   .mexit
   .endif
   .else
   .mexit
   .endif
   .else
   .mexit
   .endif
.endm
