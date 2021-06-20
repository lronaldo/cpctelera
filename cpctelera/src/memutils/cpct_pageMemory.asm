;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2017 Augusto Ruiz / RetroWorks (@augurui)
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
.module cpct_memutils

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_pageMemory
;;
;;    Makes accesible a memory zone from the upper 64Kb on a standard CPC 6128 or 
;; from a memory extension.
;;
;; C Definition:
;;    void cpct_pageMemory (<u8> *configAndBankValue*);
;;
;; Input Parameters (1 byte):
;;  (1B A) configAndBankValue   - RAM pages configuration
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_pageMemory_asm
;;
;; Details:
;;    This function modifies the memory layout. The memory address is changed 
;; based on the current configuration, and depending on the configuration, 
;; extended memory can be accessed using 16 bit addresses. 
;; Due to the address bus size, the address only represents 64Kb of data. In 
;; order to use memory higher than the first 64Kb, memory must be "paged-in". 
;; The process of paging the memory can also be used to alter the order of the 
;; pages, whether in the first 64Kb of data or higher.
;;
;;    Memory layout is defined by: The RAM pages configuration and the memory
;; bank we will use to page. There are 8 possible RAM configurations, shown 
;; below:
;; 
;; (start code) 
;; -Address-     0      1      2      3      4      5      6      7
;; 0000-3FFF   RAM_0  RAM_0  RAM_4  RAM_0  RAM_0  RAM_0  RAM_0  RAM_0
;; 4000-7FFF   RAM_1  RAM_1  RAM_5  RAM_3  RAM_4  RAM_5  RAM_6  RAM_7
;; 8000-BFFF   RAM_2  RAM_2  RAM_6  RAM_2  RAM_2  RAM_2  RAM_2  RAM_2
;; C000-FFFF   RAM_3  RAM_7  RAM_7  RAM_7  RAM_3  RAM_3  RAM_3  RAM_3
;; (end code)
;;
;;    Default configuration is the 0 config, where only the first 64Kb of 
;; memory are accessed.
;;
;;    The actual pages represented by RAM0-RAM7 depend on the bank number.
;; The bank number is 0 for the first 64Kb in the expansion, 1 for the next 
;; 64Kb, and so on.
;;
;;    The Video RAM is always located in the first 64K, VRAM is in no way affected 
;; by this register.
;;
;;    For example, it is typical to bank using the 0x4000 memory address as target.
;; In this scenario the configurations 0, 4, 5, 6 and 7 are used in conjunction with
;; the BANK 0, which is the first 64 Kb of memory, because you want the selected 
;; RAM page to be loaded in the first 64Kb.
;;
;;    In order to easily use the banks, there are macro definitions available to
;; use either in C or in assembly (see <Memory Pagination Macros>). For C users, 
;; they are directly available when one of these headers is included: *cpctelera.h*,
;; *memutils/banks.h*. Assembly users may include *memutils/cpct_setBank_const.asm*.
;;
;; Destroyed Register values: 
;;    AF, BC
;;
;;    L (when called through C-interface)
;;
;; Required memory:
;;    C-binding   - 9 bytes
;;    ASM-binding - 8 bytes
;;
;; Time Measures:
;; (start code)
;;   Case      | microSecs (us) | CPU Cycles |
;; -------------------------------------------
;;    Any      |       13       |    52      |
;; -------------------------------------------
;; Asm saving  |       -1       |    -4      |
;; -------------------------------------------
;; (end code)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

pageMemory:
  or   #0xC0		    ;; [2] Use Gate Array function 3: Ram Banking (0b11-------).
                    ;;     Rest of A register holds our desired memory bank configuration
  ld   bc, #0x7F00  ;; [3] We are going to use port 0x7F (Gate Array, for memory configuration)
  out  (c), a       ;; [4] Submit this order to Gate Array
  
  ret               ;; [3] Then Return