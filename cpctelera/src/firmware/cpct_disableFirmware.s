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

;;================== cpct_mode_rom_status ===================================
;; Store the last selection of MODE, INT.GENERATOR and ROM status 
;;  Default: 0x9C = (10011100) == (GGGIRRnn)
;;  GGG=Command for video mode and ROM selection (100)
;;  I=Interrupt Generation Enabled (1)
;;  RR=Reading from Lower and Upper ROM Disabled (11) (a 0 value means ROM enabled)
;;  nn=Video Mode 1 (01)
cpct_mode_rom_status:: .db #0x9D

;; Reserve two bytes (a word) to store the address where ROM routines start
;; This will be used by functions _cpct_disableFirmware and _cpct_reenableFirmware
cpct_firmware_address:: .DW 0

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
_cpct_disableFirmware::
   DI                         ;; [ 4] Disable interrupts
   LD HL,(firmware_RST_jp)    ;; [16] Obtain firmware ROM code pointer and store it for restoring it later
   LD (cpct_firmware_address),HL ;; [16]

   IM 1                       ;; [ 8] Set Interrupt Mode 1 (CPU will jump to &0038 when a interrupt occurs)
   LD HL,#0xC9FB              ;; [10] FB C9 (take into account little endian) => EI : RET

   LD (firmware_RST_jp), HL   ;; [16] Setup new "interrupt handler" and enable interrupts again
   EI                         ;; [ 4] 

   RET                        ;; [10]
