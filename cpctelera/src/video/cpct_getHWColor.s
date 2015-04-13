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
;## FUNCTION: _cpct_getHWColor                                        ###
;########################################################################
;### Returns the hardware colour value (0-31) for a given firmware    ###
;### colour value (0-26). The colour value returned can be used by    ###
;### other palette functions requiring hardware colour values.        ###
;########################################################################
;### INPUTS (1 Byte)                                                  ###
;###  * (1B C) Firmware INK colour value to convert (0-26)            ### 
;########################################################################
;### RETURN VALUE                                                     ###
;###  Returns a byte value, 0-31, with the hardware colour value      ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: BC, HL                               ###
;########################################################################
;### MEASURED TIME                                                    ###
;###   67 cycles (16.75 ns)                                           ###
;########################################################################
;

;; Colour table
firmware2hw_colour:  .db 0x14, 0x04, 0x15, 0x1C, 0x18, 0x1D, 0x0C, 0x05, 0x0D
                     .db 0x16, 0x06, 0x17, 0x1E, 0x00, 0x1F, 0x0E, 0x07, 0x0F
                     .db 0x12, 0x02, 0x13, 0x1A, 0x19, 0x1B, 0x0A, 0x03, 0x0B

_cpct_getHWColour::
   LD  HL, #2               ;; [10] HL = SP + 2 (Place where parameters start) 
   ADD HL, SP               ;; [11]
   LD   C, (HL)             ;; [ 7] A = Firmware INK colour value 

   LD  HL, #firmware2hw_colour ;; [10] HL points to the start of the colour table
   LD   B, #0               ;; [ 7] B = 0, to be able to use C as an incremente for HL (adding BC)
   ADD HL, BC               ;; [11] HL += C (as B=0), HL points to the exact hardware color value to return

   LD   L, (HL)             ;; [ 7] L = Return value (hardware colour for firmware colour supplied)
   LD   H, B                ;; [ 4] H = 0, to leave HL just with the value of L

   RET                      ;; [10] Return