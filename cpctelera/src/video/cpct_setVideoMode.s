;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_videomode

.include /videomode.s/

;
;########################################################################
;## FUNCTION: _cpct_setVideoMode                                      ###
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
;###  * (1B A) Video mode to set (0-3:only lowest 2 bits will be used)###
;###   WARNING: If parameter is >3, unexpected results my happen.     ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, HL                           ###
;########################################################################
;### MEASURED TIME                                                    ###
;###  71 cycles (17.75 us)                                            ###
;########################################################################
;### CREDITS:                                                         ###
;###  This function was coded copying and modifying cpc_setMode from  ###
;### cpcrslib by Raul Simarro.                                        ###
;########################################################################
;
_cpct_setVideoMode::
   ;; Get Parameter (Mode to select) from stack
   LD   HL, #2               ;; [10] HL = SP + 2 (Place where parameters are in the stack)
   ADD  HL, SP               ;; [11]
   LD   C, (HL)              ;; [ 7] A = First Paramter (Video Mode to be selected)

   LD   HL, #gfw_mode_rom_status ;; [10] HL points to present MODE, INT.GEN and ROM selection byte.
   LD   A, (HL)              ;; [ 7] A = Present values for MODE, INT.GEN and ROM selection. (See mode_rom_status)
   AND #0xFC                 ;; [ 7] A = (xxxxxx00) set bits 1,0 to 0, to prepare them for inserting the mode parameter
   OR   C                    ;; [ 4] A = Mixes previously selected ING.GEN and ROM values with user selected MODE (last 2 bits)
   LD   B,  #GA_port_byte    ;; [ 7] B = Gate Array Port (0x7F)
   OUT (C), A                ;; [12] GA Command: Set Video Mode

   LD (HL), A                ;; [ 7] Save new Mode and ROM status for later use if required

   RET                       ;; [10] Return
