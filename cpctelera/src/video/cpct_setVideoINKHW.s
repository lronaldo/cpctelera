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
;## FUNCTION: _cpct_setVideoINKHW                                     ###
;########################################################################
;### This function modifies video palette registers to set the INK    ###
;### of one of the available PENs of the Amstrad CPC. Through the     ###
;### Gate Array Port (&7F) it accesses the PAL chip and changes the   ###
;### required INK colour register. The parameter color value must be  ###
;### one of the valid hardware color numbers (not firmware ones).     ###
;########################################################################
;### INPUTS (3 Bytes)                                                 ###
;###  * (1B C) Number of the PEN colour to change (0-16)              ### 
;###  * (1B A) foreground INK hardware colour to be set (INKR)        ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, HL                           ###
;########################################################################
;### MEASURED TIME                                                    ###
;###   95 cycles (23.75 ns)                                           ###
;########################################################################
;### CREDITS:                                                         ###
;###   This function has been constructed with great help from the    ###
;### documentation at Grimware about the Gate Array.                  ###
;### http://www.grimware.org/doku.php/documentations/devices/gatearray###
;########################################################################
;
_cpct_setVideoINKHW::
   LD  HL, #2               ;; [10] HL = SP + 2 (Place where parameters start) 
   ADD HL, SP               ;; [11]
   LD   C, (HL)             ;; [ 7] C = First Parameter (PEN)
   INC  HL                  ;; [ 6] 
   LD   A, (HL)             ;; [ 7] A = Second Parameter (INKR)

  ;OR   #PAL_PENR           ;; [ 7] (CCCnnnnn) Mix 3 bits for PENR command (C) and 5 for PEN number (n). As PENR command is 000, nothing to be done here.
   LD   B, #GA_port_byte    ;; [ 7] B = Gate Array Port (0x7F). C has the command that GA will execute.
   OUT (C), C               ;; [12] GA command: Select PENR. C = Command + Parameter (PENR + PEN number to select)

   OR   #PAL_INKR           ;; [ 7] (CCCnnnnn) Mix 3 bits for INKR command (C) and 5 for INKR number (n). 
   OUT (C), A               ;; [11] GA command: Set INKR. A = Command + Parameter (INKR + INK to be set for selected PEN number)

   RET                      ;; [10] Return