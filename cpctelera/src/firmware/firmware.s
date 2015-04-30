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

;;
;; Constant values
;;
.equ firmware_RST_jp, 0x38  ;; Memory address were a jump (jp) to the firmware code is stored.
.equ GA_port_byte,    0x7F  ;; 8-bit Port of the Gate Array

;;
;; Global symbols
;;

;;
;; Variable: cpct_firmware_address
;;
;;    16-bit space for storing the address where ROM routines start. This is
;; used by functions <cpct_disableFirmware> and <cpct_reenableFirmware>.
;;
.globl _cpct_firmware_address

;;
;; Variable: cpct_mode_rom_status
;;
;;    8-bit space for storing the latest selection of MODE, INT.GENERATOR 
;; and ROM status. This lets us know if Lower / Upper ROM are enabled or 
;; disabled, if interrupts are enabled or not, and which Video Mode is actually set.
;;
;; Description of the bits:
;;  > [ GGGIRRnn ]
;;    GGG - Command for video mode and ROM selection (100)
;;    I   - Interrupt Generation Enabled (1)
;;    RR  - Reading from Lower and Upper ROM Disabled (11) (a 0 value means ROM enabled)
;;    nn  - Video Mode 1 (01)
;;
;; Default value: 0x9C = (10011100)
;;
.globl _cpct_mode_rom_status
