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
;### MODULE: Firmware Enable/Disable                               ###
;#####################################################################
;### Routines to disable CPC Firmware and reenable it when needed  ###
;#####################################################################
;

;
;########################################################################
;### FUNCTION: _cpct_disableFirmware                                  ###
;########################################################################
;###  Disables the firmware modifying the interrupt vector at 0x38.   ###
;### Normally, firmware routines are called and executed at each      ###
;### interrupt and the ROM entry point is stored at 0x38.             ###
;### This function substitutes the 2 bytes located at 0x38 by 0xC9FB, ###
;### (FB = EI, C9 = RET), which basically does nothing at each        ###
;### interruption.                                                    ###
;########################################################################
;### INPUTS (none)                                                    ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values:HL                                    ###
;########################################################################
;### MEASURED TIME                                                    ###
;###  84 cycles                                                       ###
;########################################################################
;### CREDITS:                                                         ###
;###  This function was coded copying and modifying                   ###
;### cpc_disableFirmware from cpcrslib by Raul Simarro.               ###
;########################################################################
;
.globl _cpct_disableFirmware
_cpct_disableFirmware::
   DI                         ;; [ 4c] Disable interrupts
   LD HL,(#0x38)              ;; [16c] Obtain firmware ROM code pointer and store it for restoring it later
   LD (firmware_address),HL   ;; [16c]

   IM 1                       ;; [ 8c] Set Interrupt Mode 1 (CPU will jump to &0038 when a interrupt occrs)
   LD HL,#0xC9FB              ;; [10c] FB C9 (take into account little endian) => EI : RET

   LD (#0x38), HL             ;; [16c] Setup new "interrupt handler" and enable interrupts again
   EI                         ;; [ 4c] 

   RET                        ;; [10c]

;; Reserve two bytes (a word) to store the address where ROM routines start
;; This will be used by functions _cpct_disableFirmware and _cpct_reenableFirmware
firmware_address: .DW 0

;
;########################################################################
;### FUNCTION: _cpct_reenableFirmware                                 ###
;########################################################################
;###   Restores the ROM address where the firmware routines start.    ###
;### Beware: cpct_disableFirmware has to be called before calling     ###
;### this routine; otherwise, it would put a 0 in 0x38 and a reset    ###
;### will be performed at the next interrupt.                         ###
;########################################################################
;### INPUTS (none)                                                    ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: HL                                   ###
;########################################################################
;### MEASURED TIME                                                    ###
;###  50 cycles                                                       ###
;########################################################################
;### CREDITS:                                                         ###
;###  This function was coded copying and modifying                   ###
;### cpc_disableFirmware from cpcrslib by Raul Simarro.               ###
;########################################################################
;
.globl _cpct_reenableFirmware
_cpct_reenableFirmware::
   DI                         ;; [ 4c] Disable interrupts

   LD HL, (firmware_address)  ;; [16c] Restore previously saved pointer to ROM code
   LD (#0x38), HL             ;; [16c]

   EI                         ;; [ 4c] Reenable interrupts and return
   RET                        ;; [10c]
