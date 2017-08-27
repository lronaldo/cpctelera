;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2017 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;; File: Math Macros
;;
;;    Useful assembler macros for doing common math operations
;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Macro: add_REGPAIR_a 
;;
;;    Performs the operation REGPAIR = REGPAIR + A. REGPAIR is any given pair of 8-bit registers.
;;
;; Parameters:
;;    RH    - Register 1 of the REGPAIR. Holds higher-byte value
;;    RL    - Register 2 of the REGPAIR. Holds lower-byte value
;; 
;; Input Registers: 
;;    RH:RL - 16-value used as left-operand and final storage for the sum
;;    A     - Second sum operand
;;
;; Return Value:
;;    RH:RL - Holds the sum of RH:RL + A
;;
;; Details:
;;    This macro performs the sum of RH:RL + A and stores it directly on RH:RL.
;; It uses only RH:RL and A to perform the operation.
;;
;; Modified Registers: 
;;    A, RH, RL
;;
;; Required memory:
;;    5 bytes
;;
;; Time Measures:
;; (start code)
;;  Case | microSecs(us) | CPU Cycles
;; ------------------------------------
;;  Any  |       5       |     20
;; ------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.macro add_REGPAIR_a rh, rl
   ;; First Perform RH = E + A
   add rl    ;; [1] A' = RL + A 
   ld  rl, a ;; [1] RL' = A' = RL + A. It might generate Carry that must be added to RH
   
   ;; Then Perform RH = RH + Carry 
   adc rh    ;; [1] A'' = A' + RH + Carry = RL + A + RH + Carry
   sub rl    ;; [1] Remove RL'. A''' = A'' - RL' = RL + A + RH + Carry - (RL + A) = RH + Carry
   ld  rh, a ;; [1] Save into RH (RH' = A''' = RH + Carry)
.endm

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Macro: add_de_a
;;
;;    Performs the operation DE = DE + A
;;
;; Parameters:
;;    None
;; 
;; Input Registers: 
;;    DE    - First sum operand and Destination Register
;;    A     - Second sum operand
;;
;; Return Value:
;;    DE - Holds the sum of DE + A
;;
;; Details:
;;    This macro performs the sum of DE + A and stores it directly on DE.
;; It uses only DE and A to perform the operation.
;;    This macro is a direct instantiation of the macro <add_REGPAIR_a>.
;;
;; Modified Registers: 
;;    A, DE
;;
;; Required memory:
;;    5 bytes
;;
;; Time Measures:
;; (start code)
;;  Case | microSecs(us) | CPU Cycles
;; ------------------------------------
;;  Any  |       5       |     20
;; ------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.macro add_de_a
   add_REGPAIR_a  d, e
.endm

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Macro: add_hl_a
;;
;;    Performs the operation HL = HL + A
;;
;; Parameters:
;;    None
;; 
;; Input Registers: 
;;    HL    - First sum operand and Destination Register
;;    A     - Second sum operand
;;
;; Return Value:
;;    HL - Holds the sum of HL + A
;;
;; Details:
;;    This macro performs the sum of HL + A and stores it directly on HL.
;; It uses only HL and A to perform the operation.
;;    This macro is a direct instantiation of the macro <add_REGPAIR_a>.
;;
;; Modified Registers: 
;;    A, HL
;;
;; Required memory:
;;    5 bytes
;;
;; Time Measures:
;; (start code)
;;  Case | microSecs(us) | CPU Cycles
;; ------------------------------------
;;  Any  |       5       |     20
;; ------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.macro add_hl_a
   add_REGPAIR_a  d, e
.endm

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Macro: add_bc_a
;;
;;    Performs the operation BC = BC + A
;;
;; Parameters:
;;    None
;; 
;; Input Registers: 
;;    BC    - First sum operand and Destination Register
;;    A     - Second sum operand
;;
;; Return Value:
;;    BC - Holds the sum of BC + A
;;
;; Details:
;;    This macro performs the sum of BC + A and stores it directly on BC.
;; It uses only BC and A to perform the operation.
;;    This macro is a direct instantiation of the macro <add_REGPAIR_a>.
;;
;; Modified Registers: 
;;    A, BC
;;
;; Required memory:
;;    5 bytes
;;
;; Time Measures:
;; (start code)
;;  Case | microSecs(us) | CPU Cycles
;; ------------------------------------
;;  Any  |       5       |     20
;; ------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.macro add_bc_a
   add_REGPAIR_a  b, c
.endm