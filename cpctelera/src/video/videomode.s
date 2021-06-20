;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;### MODULE: SetVideoMode                                          ###
;#####################################################################
;### Routines to establish and control video modes                 ###
;#####################################################################
;
.module cpct_video
   
;;
;; Constant values
;;
.equ GA_port,      0x7F00  ;; 16-bit Port of the Gate Array (for the use with BC register)
.equ GA_port_byte, 0x7F    ;; 8-bit Port of the Gate Array
.equ PAL_PENR,     0x00    ;; Command to select a PEN register in the PAL chip
.equ PAL_INKR,     0x40    ;; Command to set the INK of a previously selected PEN register in the PAL chip
.equ PPI_PORT_B,   0xF5    ;; Port B of the PPI, used to read Vsync/Jumpers/PrinterBusy/CasIn/Exp information

;;
;; External values
;;
.globl _cpct_mode_rom_status   ;; defined in cpc_mode_rom_status.s

