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
.module cpct_setVideoMemoryOffset

;
;########################################################################
;### FUNCTION: cpct_setVideoMemoryOffset                              ###
;########################################################################
;### Changes part of the address where video memory starts. Video me- ###
;### mory starts at an address that can be defined in binary as:      ###
;###    Start Addres = ppppppoooooooo00                               ###
;### where you have 6 bits that define the Page, and 8 bits that defi-###
;### ne the offset. The 2 Least Significant Bits (LSb) are allways 0. ###
;### This funcion changes the 8 bits that define the offset. This 8   ###
;### bits are controlled by Register R13 from the CRTC. If you wanted ###
;### to change the 6 Most Significant bits (MSb), aka the Page, you   ###
;### should call _cpct_setVideoMemoryPage.                            ###
;###                                                                  ###
;### Changing this effectively changes the place where your Video Me- ###
;### mory will be located in RAM, and it will change what is displa-  ###
;### yed. This could be used to produce scrolling effects or to make  ###
;### a fine grained control of double/triple buffers by hardware.     ###
;########################################################################
;### INPUTS (1 Byte)                                                  ###
;###  * (1B A) New starting offset for Video Memory                   ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: F, BC, DE                            ###
;########################################################################
;### MEASURES                                                         ###
;### MEMORY: 14 bytes                                                 ###
;### TIME:   76 cycles                                                ###
;########################################################################
;
.globl _cpct_setVideoMemoryOffset
_cpct_setVideoMemoryOffset::
   ;; Get the parameter from the stack
   LD  HL, #2        ;; [10] HL = SP + 2 (Place where parameter is)
   ADD HL, SP        ;; [11]
   LD  A, (HL)       ;; [ 7] A = First Parameter (Video Memory Offset to Establish)

   ;; Select R13 Register from the CRTC and Write there the selected Video Memory Offset
   LD  BC, #0xBC0D   ;; [10] 0xBC = Port for selecting CRTC Register, 0x0D = Selecting R13
   OUT (C), C        ;; [12] Select the R13 Register (0x0D to port 0xBC)
   INC B             ;; [ 4] Change Output port to 0xBD (B = 0xBC + 1 = 0xBD)
   OUT (C), A        ;; [12] Write Selected Video Memory Offset to R13 (A to port 0xBD)

   RET               ;; [10] Return 
