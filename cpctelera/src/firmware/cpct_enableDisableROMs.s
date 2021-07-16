;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_firmware

.include /firmware.s/

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Title: Enabling / disabling ROMs
;;
;; Functions: Enabling / disabling ROMs
;;
;; cpct_enableUpperROM  - Enables Upper ROM  [0xC000 - 0xFFFF]
;; cpct_enableLowerROM  - Enables Lower ROM  [0x0000 - 0x3FFF]
;; cpct_disableUpperROM - Disables upper ROM [0xC000 - 0xFFFF]
;; cpct_disableLowerROM - Disables Lower ROM [0x0000 - 0x3FFF]
;;
;;    This 4 functions enable / disable lower or upper ROMs.
;;
;; C Definitions:
;;    void <cpct_enableUpperROM> ()
;;
;;    void <cpct_enableLowerROM> ()
;;
;;    void <cpct_disableUpperROM> ()
;;
;;    void <cpct_enableUpperROM> ()
;;
;; Assembly calls:
;;   > call cpct_enableUpperROM_asm
;;   > call cpct_enableLowerROM_asm
;;   > call cpct_disableUpperROM_asm
;;   > call cpct_enableUpperROM_asm
;;
;; Known limitations:
;;    * If the execution of your program is going through some of these 2 
;; ROM spaces and you enable ROM, CPU will be unable to get machine code 
;; of your program, as it will start reading from ROM instead of RAM (where 
;; your program is placed). This will result in unexpected behaviour.
;;    * Enabling Lower ROM re-enables Firmware JMP in interrupt mode 1, 
;; as it re-establishes the values at memory location 0x0038. If you have 
;; disabled the firmware by putting FB:C9 (EI:RET) at 0x0038 (as 
;; <cpct_disableFirmware> does) you may experience undesired behaviour 
;; when next interrupts come. The most typical is a return to MODE 1, as 
;; firmware may be unaware of your currently selected MODE and will try to 
;; re-establish its own MODE. Take it into account and disable interrupts 
;; if you need.
;;  * This function *will not work from ROM*, as it uses self-modifying code.
;;
;; Details:
;;    These 4 functions enable / disable lower or upper ROMs. By default, *CPCtelera* 
;; sets both ROMS as disabled at the first event of video mode change. Enabling 
;; one of them means changing the way the CPU gets information from memory: on 
;; the places where ROM is enabled, CPU gets values from ROM whenever it tries 
;; to read from memory.  
;;
;; If ROM is disabled, these memory reads get values from RAM. ROMs 
;; are mapped in this address space:
;;
;;  Lower ROM - [ 0x0000 - 0x3FFF ]
;;  Upper ROM - [ 0xC000 - 0xFFFF ]
;;
;; CPU Requests to write to memory are always mapped to RAM, so there is no need
;; to worry about that. Also, Gate Array always gets video memory values from 
;; RAM (it never reads from ROM), so enabling Upper ROM does not have any impact
;; on the screen.
;;
;;
;; Destroyed Register values: 
;;    AF, BC, HL
;;
;; Required memory:
;;    33 bytes
;;
;; Time Measures:
;; (start code)
;; Case  | microSecs(us) | CPU Cycles
;; -----------------------------------
;; Best  |      23       |     92
;; -----------------------------------
;; Worst |      26       |    104
;; -----------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.globl _cpct_mode_rom_status

_cpct_enableLowerROM::
cpct_enableLowerROM_asm::
   ld   hl, #0xFBE6          ;; [3] HL = Machine Code. E6 FB = AND #0b11111011 = Reset Bit 3 (Enable Lower ROM, 0 = enabled)
   jr mrs_modifyROMstatus    ;; [3] Jump to ROM-Modification Code

_cpct_disableLowerROM::
cpct_disableLowerROM_asm::
   ld   hl, #0x04F6          ;; [3] HL = Machine Code. F6 04 = OR #0b00000100 = Set Bit 3 (Disable Lower ROM, 0 = enabled)
   jr mrs_modifyROMstatus    ;; [3] Jump to ROM-Modification Code

_cpct_enableUpperROM::
cpct_enableUpperROM_asm::
   ld   hl, #0xF7E6          ;; [3] HL = Machine Code. E6 F7 = AND #0b11110111 = Reset Bit 4 (Enable Upper ROM, 0 = enabled)
   jr mrs_modifyROMstatus    ;; [3] Jump to ROM-Modification Code

_cpct_disableUpperROM::
cpct_disableUpperROM_asm::
   ld   hl, #0x08F6          ;; [3] HL = Machine Code. F6 08 = OR #0b00001000 = Set Bit 4 (Disable Upper ROM, 0 = enabled)
   ;jp  mrs_modifyROMstatus  ;; [3] Jump to ROM-Modification Code

mrs_modifyROMstatus:
   ld  (mrs_operation), hl   ;; [5] Modify Machine Code that makes the operation (AND/OR) to set/reset ROM bits
   ld   hl, #_cpct_mode_rom_status ;; [3] HL points to present MODE, INT.GEN and ROM selection byte.
   ld    a, (hl)             ;; [2] A = mode_rom_status (present value)
mrs_operation:
   and  #0b11111011          ;; [2] bit 3 of A = 0 --> Lower ROM enabled (0 means enabled)
   ld    b, #GA_port_byte    ;; [2] B = Gate Array Port (0x7F)
   out (c), a                ;; [4] GA Command: Set Video Mode and ROM status (100)

   ld (hl), a                ;; [2] Save new Mode and ROM status for later use if required

   ret                       ;; [3] Return


