;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;; File: Opcodes
;;
;;    Constant definitions of Z80 opcodes. This will be normally used as data
;; for self-modifying code.
;;

;; Constant: opc_JR
;;    Opcode for "JR xx" instruction. Requires 1-byte parameter (xx)
opc_JR   = 0x18

;; Constant: opc_LD_D
;;    Opcode for "LD d, xx" instruction. Requires 1-byte parameter (xx)
opc_LD_D = 0x16

;; Constant: opc_EI
;;    Opcode for "EI" instruction. 
opc_EI = 0xFB

;; Constant: opc_DI
;;    Opcode for "DI" instruction. 
opc_DI = 0xF3