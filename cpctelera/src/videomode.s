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
;### MODULE: SetVideoMode                                          ###
;#####################################################################
;### Routines to establish and control video modes                 ###
;#####################################################################
;

;
;########################################################################
;### FUNCTION: _cpct_setVideoMode                                     ###
;########################################################################
;### This function establishes the video mode for the Amstrad CPC.    ###
;### Video modes available are:                                       ###
;###  0: 160x200, 16 colours                                          ###
;###  1: 320x200,  4 colours                                          ###
;###  2: 640x200,  2 colours                                          ###
;###  3: 160x200,  4 colours (undocumented)                           ###
;### Important: Do not use this method when the firmware is up and    ###
;###    running, as it modifies Upper and Lower ROM paging.           ###
;########################################################################
;### INPUTS (1 Byte)                                                  ###
;###  * (1B) Video mode to set (only lowest 2 bits will be used)      ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, HL                           ###
;########################################################################
;### MEASURED TIME                                                    ###
;###  67 cycles                                                       ###
;########################################################################
;### CREDITS:                                                         ###
;###  This function was coded copying and modifying cpc_setMode from  ###
;### cpcrslib by Raul Simarro.                                        ###
;########################################################################
;
.globl _cpct_setVideoMode
_cpct_setVideoMode::
   LD  HL, #2        ;; [10c] Load A with the parameter (video mode)
   ADD HL, SP        ;; [11c]
   LD   A, (HL)      ;; [ 7c]

   OR   A, #140      ;; [ 7c] 140 = @10001100 => Mode  and  rom  selection  (and Gate Array function)
   LD  BC, #0x7F00   ;; [10c] 7F00h           => Port of the Gate Array Chip
   OUT (C),A         ;; [12c] Send request to Port

   RET               ;; [10c] Return