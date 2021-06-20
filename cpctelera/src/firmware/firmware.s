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
;#####################################################################
;### MODULE: Firmware and ROM routines                             ###
;#####################################################################
;### Routines to disable CPC Firmware and reenable it when needed, ###
;### and managing Upper and Lower ROMs.                            ###
;#####################################################################
;
.module cpct_firmware

;;
;; Title: Firmware&ROM constants
;;

;;
;; Constants: Firmware useful constants
;;
;;    Constants used by firmware routines.
;;
;;    firmware_RST_jp - Memory address that stores a pointer to the start of 
;; firmware code, executed on every interruption.
;;    GA_port_byte    - Output port where Gate Array (GA) listens.
;;
.equ firmware_RST_jp, 0x38  ;; Memory address were a jump (jp) to the firmware code is stored.
.equ GA_port_byte,    0x7F  ;; 8-bit Port of the Gate Array