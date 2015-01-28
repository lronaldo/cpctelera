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

;;
;; Constant values
;;
.equ GA_port, 0x7F00    ;; 16-bit Port of the Gate Array (for the use with BC register)
.equ GA_port_byte, 0x7F ;; 8-bit Port of the Gate Array
.equ PAL_PENR, 0x00     ;; Command to select a PEN register in the PAL chip
.equ PAL_INKR, 0x40     ;; Command to set the INK of a previously selected PEN register in the PAL chip
;
;########################################################################
;## FUNCTION: _cpct_setVideoMode                                     ###
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
;###  64 cycles (0.016 ms)                                            ###
;########################################################################
;### CREDITS:                                                         ###
;###  This function was coded copying and modifying cpc_setMode from  ###
;### cpcrslib by Raul Simarro.                                        ###
;########################################################################
;
.globl _cpct_setVideoMode
_cpct_setVideoMode::
   LD  HL, #2              ;; [10] HL = SP + 2 (Place where parameters are in the stack)
   ADD HL, SP              ;; [11]
   LD   A, (HL)            ;; [ 7] A = First Paramter (Video Mode to be selected)

   OR   #0x8C              ;; [ 7] A = (GGGIRRnn) Mix 4 things into A: GGG=Command for video mode selection (100),
                           ;;   0x8C = (10001100) I = Interrupt Generation Disabled (0), RR=Upper and lower ROM disabled (11),
                           ;;                     nn= Video mode wanted (user parameter, A) 
   LD   B, #GA_port_byte   ;; [ 7] B = Gate Array Port (0x7F)
   OUT (C), A              ;; [12] GA Command: Set Video Mode

   RET                     ;; [10] Return

;
;########################################################################
;## FUNCTION: _cpct_setVideoPalette                                   ###
;########################################################################
;### This function modifies video palette registers to set the INKs   ###
;### colours to use. Palette registers are contained inside the PAL   ###
;### chip of the CPC (which is located inside the Gate Array). To     ###
;### access PAL chip, the same port (&7F) as Gate Array is used.      ###
;### The function is not thought to be optimal, but secure. If a more ###
;### optimal one is required, it can be derived from this code.       ###
;###                                                                  ###
;### Known limitations: this function cannot set blinking colors, as  ###
;### blinking colours are managed by the firmware (not by hardware)   ###
;########################################################################
;### INPUTS (3 Bytes)                                                 ###
;###  * (2B DE) Pointer to an array of bytes containing colour numbers### 
;###  * (1B A) Number of colours to set (up to 16)                    ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURED TIME                                                    ###
;### 101 + 78*C (C = number of colours to be set, 2nd parameter)      ###
;### Example:                                                         ###
;###  16 colours = 1349 cycles (0.337 ms)                             ###
;########################################################################
;### CREDITS:                                                         ###
;###   This function has been constructed with great help from the    ###
;### documentation at Grimware about the Gate Array.                  ###
;### http://www.grimware.org/doku.php/documentations/devices/gatearray###
;########################################################################
;
.globl _cpct_setVideoPalette
_cpct_setVideoPalette::
   ;; Getting parameters from stack
   LD  HL, #2               ;; [10] HL = SP + 2 (Place where parameters start) 
   ADD HL, SP               ;; [11]
   LD   E, (HL)             ;; [ 7] DE = First parameter, pointer to the array that contains desired color IDs 
   INC  HL                  ;; [ 6]      (should get it in little endian order, E, then D)
   LD   D, (HL)             ;; [ 7]
   INC  HL                  ;; [ 6]
   LD   A, (HL)             ;; [ 7] A = Second parameter, Number of colours to set (up to 16) 

   EX   DE, HL              ;; [ 4] HL = DE, We will use HL to point to the array and get colour ID values
   DEC  A                   ;; [ 4] A -= 1 (We leave A as 1 colour less to convert 16 into 15 and be able to use AND to ensure no more than 16 colours are passed)
   AND  #0x0F               ;; [ 7] A %= 16, A will be 15 at most, that is, the number of colours to set minus 1
   INC  A                   ;; [ 4] A += 1 Restore the 1 to leave A as the number of colours to set
   LD   E, A                ;; [ 4] E = A, E will have the number of colours to set 
   LD   D, #0               ;; [ 7] D = 0, D will count from 0 to E 

   LD   B, #GA_port_byte    ;; [ 7] BC = Gate Array Port (0x7F), to send commands using OUT
svp_setPens_loop:
   LD   A, D                ;; [ 4] A = D, A has index of the next PEN to be established
  ;OR   #PAL_PENR           ;; [ 7] A = (CCCnnnnn) Mix 3 bits for PENR command (C) and 5 for PEN number (n). As PENR command is 000, nothing to be done here.
   OUT (C),A                ;; [11] GA Command: Select PENR. A = Command + Parameter (PENR + PEN number to select)

   LD   A, (HL)             ;; [ 7] Get Color value (INK) for the selected PEN from array
   AND  #0x1F               ;; [ 7] Leave out only the 5 Least significant bits (3 MSB can cause problems)
   OR   #PAL_INKR           ;; [ 7] (CCCnnnnn) Mix 3 bits for INKR command (C) and 5 for PEN number (n). 
   OUT (C),A                ;; [11] GA Command: Set INKR. A = Command + Parameter (INKR + INK to be set for selected PEN number)

   INC  HL                  ;; [ 6] HL += 1, Point to the next INK in the array
   INC  D                   ;; [ 4] D += 1, Next PEN index to be set 
   DEC  E                   ;; [ 4] E -= 1, count how many PENs still to be set
   JP NZ, svp_setPens_loop  ;; [10] If more than 0 PENs to be set, continue

   RET                      ;; [10] Return

;
;########################################################################
;## FUNCTION: _cpct_setVideoINK                                       ###
;########################################################################
;### This function modifies video palette registers to set the INK    ###
;### of one of the available PENs of the Amstrad CPC. Through the     ###
;### Gate Array Port (&7F) it accesses the PAL chip and changes the   ###
;### required INK colour register.                                    ###
;########################################################################
;### INPUTS (3 Bytes)                                                 ###
;###  * (1B C) Number of the PEN colour to change (0-16)              ### 
;###  * (1B A) foreground INK colour to be set (INKR)                 ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, HL                           ###
;########################################################################
;### MEASURED TIME                                                    ###
;###   95 cycles (0.023 ms)                                           ###
;########################################################################
;### CREDITS:                                                         ###
;###   This function has been constructed with great help from the    ###
;### documentation at Grimware about the Gate Array.                  ###
;### http://www.grimware.org/doku.php/documentations/devices/gatearray###
;########################################################################
;
.globl _cpct_setVideoINK
_cpct_setVideoINK::
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