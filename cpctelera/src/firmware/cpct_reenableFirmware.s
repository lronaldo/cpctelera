;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;#####################################################################
;### MODULE: Firmware and ROM routines                             ###
;#####################################################################
;### Routines to disable CPC Firmware and reenable it when needed, ###
;### and managing Upper and Lower ROMs.                            ###
;#####################################################################
;
.module cpct_firmware

.include /firmware.s/

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_reenableFirmware
;;
;;    Re-enables previously disabled Amstrad CPC firmware.
;;
;; C Definition:
;;    void <cpct_reenableFirmware> (<u16> firmware_ROM_pointer) __z88dk_fastcall;
;;
;; Assembly call:
;;    > call cpct_reenableFirmware_asm
;;
;; Input Parameters (2 Bytes):
;;   (2B HL) *firmware_ROM_pointer* - 2 bytes with previous pointer stored at 0x0039.
;; This is the address where firmware ROM code starts.
;;
;; Parameter Restrictions:
;;    * *firmware_ROM_pointer* is a 16bits value that should have been previously obtained
;; calling <cpct_disableFirmware> or <cpct_removeInterruptHandler>. This 16bits value should
;; be the pointer to the firmware ROM code that must be called every time an interrupt happens.
;; This pointer is placed at 0x0039, along with a JP instruction (0xC3) at 0x0038. Being the
;; CPU in interrupt mode 1, a jump to 0x0038 address is produced 6 times per frame, 300
;; times per second.
;;
;; Details:
;;    Restores normal operation of Amstrad CPC firmware after having been disabled.
;; Do not try to call this function before disabling firmware. If you do, the most
;; normal result is getting your Amstrad CPC resetted.
;;
;;    This function could also be used to change the present interrupt handler. However,
;; take into account that it does not create a safe wrapper for the given interrupt
;; handler. Use it with care in this case.
;;
;; Destroyed Register values: 
;;    HL
;;
;; Required memory:
;;    11 bytes
;;
;; Time Measures:
;; (start code)
;; Case | microSecs(us) | CPU Cycles
;; -----------------------------------
;; Any  |      16       |     64
;; -----------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.equ JP_opcode, 0xC3

_cpct_reenableFirmware::
cpct_reenableFirmware_asm::
   di                         ;; [1] Disable interrupts
   ld a, #JP_opcode           ;; [2] A = 0xC3, opcode for JP instruction
   ld (firmware_RST_jp), a    ;; [4] Put JP instruction at 0x0038, to create a jump to the pointer at 0x0039
   ld (firmware_RST_jp+1), hl ;; [5] HL = previous interrupt handler pointer (firmware ROM pointer)
   ei                         ;; [1] Reenable interrupts and return
   ret                        ;; [3] Return
