;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014-2017 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_memutils

BANK_NUM_SHIFT					= 3

;; BANK_0: RAM_4 -> 10000-13FFF, RAM_5 -> 14000-17FFF, RAM_6 -> 18000-1BFFF, RAM_7 -> 1C000-1FFFF
BANK_0							= (0 << BANK_NUM_SHIFT)
;; BANK_1: RAM_4 -> 20000-23FFF, RAM_5 -> 24000-27FFF, RAM_6 -> 28000-1BFFF, RAM_7 -> 2C000-1FFFF
BANK_1							= (1 << BANK_NUM_SHIFT)
;; BANK_2: RAM_4 -> 30000-33FFF, RAM_5 -> 34000-37FFF, RAM_6 -> 38000-3BFFF, RAM_7 -> 3C000-3FFFF
BANK_2							= (2 << BANK_NUM_SHIFT)
;; BANK_3: RAM_4 -> 40000-43FFF, RAM_5 -> 44000-47FFF, RAM_6 -> 48000-4BFFF, RAM_7 -> 4C000-4FFFF
BANK_3							= (3 << BANK_NUM_SHIFT)
;; BANK_4: RAM_4 -> 50000-53FFF, RAM_5 -> 54000-57FFF, RAM_6 -> 58000-5BFFF, RAM_7 -> 5C000-5FFFF
BANK_4							= (4 << BANK_NUM_SHIFT)
;; BANK_5: RAM_4 -> 60000-63FFF, RAM_5 -> 64000-67FFF, RAM_6 -> 68000-6BFFF, RAM_7 -> 6C000-6FFFF
BANK_5							= (5 << BANK_NUM_SHIFT)
;; BANK_6: RAM_4 -> 70000-73FFF, RAM_5 -> 74000-77FFF, RAM_6 -> 78000-7BFFF, RAM_7 -> 7C000-7FFFF
BANK_6							= (6 << BANK_NUM_SHIFT)
;; BANK_7: RAM_4 -> 80000-83FFF, RAM_5 -> 84000-87FFF, RAM_6 -> 88000-8BFFF, RAM_7 -> 8C000-8FFFF
BANK_7							= (7 << BANK_NUM_SHIFT)

;; RAMCFG_0: 0000-3FFF -> RAM_0, 4000-7FFF -> RAM_1, 8000-BFFF -> RAM_2, C000-FFFF -> RAM_3
RAMCFG_0						= 0
;; RAMCFG_1: 0000-3FFF -> RAM_0, 4000-7FFF -> RAM_1, 8000-BFFF -> RAM_2, C000-FFFF -> RAM_7
RAMCFG_1						= 1
;; RAMCFG_2: 0000-3FFF -> RAM_4, 4000-7FFF -> RAM_5, 8000-BFFF -> RAM_6, C000-FFFF -> RAM_7
RAMCFG_2						= 2
;; RAMCFG_3: 0000-3FFF -> RAM_0, 4000-7FFF -> RAM_3, 8000-BFFF -> RAM_2, C000-FFFF -> RAM_7
RAMCFG_3						= 3
;; RAMCFG_4: 0000-3FFF -> RAM_0, 4000-7FFF -> RAM_4, 8000-BFFF -> RAM_2, C000-FFFF -> RAM_3
RAMCFG_4						= 4
;; RAMCFG_5: 0000-3FFF -> RAM_0, 4000-7FFF -> RAM_5, 8000-BFFF -> RAM_2, C000-FFFF -> RAM_3
RAMCFG_5						= 5
;; RAMCFG_6: 0000-3FFF -> RAM_0, 4000-7FFF -> RAM_6, 8000-BFFF -> RAM_2, C000-FFFF -> RAM_3
RAMCFG_6						= 6
;; RAMCFG_7: 0000-3FFF -> RAM_0, 4000-7FFF -> RAM_7, 8000-BFFF -> RAM_2, C000-FFFF -> RAM_3
RAMCFG_7						= 7

DEFAULT_MEM_CFG					= RAMCFG_0 | BANK_0
