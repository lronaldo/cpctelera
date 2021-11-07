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
;;    firmware_RST_jp       - Memory address that stores a pointer to the start of firmware code,
;;                            executed on every interruption.
;;    firmware_disc_mem_ptr - Memory address that stores a pointer to the start of the memory area
;;                            used by disc ROM.
;;    GA_port_byte          - Output port where Gate Array (GA) listens.
;;
;;
;;
;; Constants: Firmware jumpblock calls
;;
;; Memory adresses of some firmware functions.
;;
;;    firmware_cas_in_open      - Opens an input buffer and reads the first block of a file.
;;    firmware_cas_in_direct    - Reads an entire file directly into memory.
;;    firmware_cas_in_close     - Closes an input file.
;;    firmware_mc_start_program - Run a foreground program.
;;    firmware_kl_rom_walk      - Enable all background ROMs.
;;
;;

.equ firmware_RST_jp,         0x38  ;; Memory address were a jump (jp) to the firmware code is stored.
.equ firmware_disc_mem_ptr, 0xBE7D  ;; Memory address storing a pointer to disc ROM memory area.
.equ GA_port_byte,            0x7F  ;; 8-bit Port of the Gate Array.


.equ firmware_cas_in_open,      0xBC77  ;; Opens an input buffer and reads the first block of a file.
.equ firmware_cas_in_direct,    0xBC83  ;; Reads an entire file directly into memory.
.equ firmware_cas_in_close,     0xBC7A  ;; Closes an input file.
.equ firmware_mc_start_program, 0xBD16  ;; Run a foreground program.
.equ firmware_kl_rom_walk,      0xBCCB  ;; Enable all background ROMs.
