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
;;    Reenables previously disabled Amstrad CPC firmware.
;;
;; C Definition:
;;    void <cpct_reenableFirmware> ()
;;
;; Assembly call:
;;    > call cpct_reenableFirmware_asm
;;
;; Details:
;;    Restores normal operation of Amstrad CPC firmware after having been disabled.
;; Do not try to call this function before disabling firmware. If you do, the most
;; normal result is getting your Amstrad CPC resetted.
;;
;; Destroyed Register values: 
;;    HL
;;
;; Required memory:
;;    9 bytes
;;
;; Time Measures:
;; (start code)
;; Case | Cycles | microSecs (us)
;; -------------------------------
;; Any  |   50   |   14.50
;; -------------------------------
;; (end code)
;;
;; Credits:                                                       
;;    This function was coded copying and modifying cpc_disableFirmware 
;; from CPCRSLib by Raul Simarro.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_reenableFirmware::
cpct_reenableFirmware_asm::   ;; Assembly entry point
   di                         ;; [ 4] Disable interrupts

   ld   hl,(_cpct_firmware_address) ;; [16] Restore previously saved pointer to ROM code
   ld (firmware_RST_jp), hl   ;; [16]

   ei                         ;; [ 4] Reenable interrupts and return
   ret                        ;; [10]
