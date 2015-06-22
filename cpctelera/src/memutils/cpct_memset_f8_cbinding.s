;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_memutils

.include /memutils.s/

;;
;; C Binding for cpct_memset_f8
;;

_cpct_memset_f8::
   di                            ;; [ 4] Disable interrupts first
   ld   (msf8_restoreSP + 1), sp ;; [20]

   ;; Recover parameters from stack
   pop  hl                       ;; [10] HL = Return address
   pop  hl                       ;; [10] HL = Array pointer
   pop  de                       ;; [10] DE = value to be set
   pop  bc                       ;; [10] BC = Size of the array
                                 ;; No need to restore them, as sp will be directly restored later on

.include /cpct_memset_f8.asm/      ;; Include function code