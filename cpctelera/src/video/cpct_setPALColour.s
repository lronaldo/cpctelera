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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Function: cpct_setPALColour
;;
;;   Changes one colour value of the Palette, similarly to BASIC's INK instruction.
;;
;; C Definition:
;;    void <cpct_setPALColour> (<u8> *pen*, <u8> *hw_ink*)
;;
;; Input Parameters (2 Bytes):
;;    (1B C) pen    - [0-16] Index of the palette colour to change. Similar to PEN Number in BASIC.
;;    (1B A) hw_ink - [0-31] New hardware colour value for the given palette index.
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_setPALColour_asm
;;
;; Parameter Restrictions:
;;    - *pen* must be <= 16. Values [0-15] are normal palette colours, whilest index 16
;; refers to the Screen Border. Giving pen > 16 may yield unexpected behaviour.
;;    - *hw_ink* must be <= 31.
;;
;; Requirements:
;;    This function requires the CPC *firmware* to be *DISABLED*. Otherwise, it
;; may not work, as firmware tends to restore palette colour values to its own
;; selection.
;;
;; Details:
;;    This function modifies changes one concrete palette registers to set the
;; hardware value of one of the available colours. This process is similar to the
;; one that does the BASIC's INK command. However, this function does it giving a
;; direct command to the PAL chip (which is inside the Gate Array). The PAL chip
;; contains the 17 registers that hold the colour values for the 16 available
;; colours of the Amstrad CPC (PENs), and the screen border colour.
;;
;; Gate Array (GA) is accessed through the port 0x7F. PENR and INKR commands are
;; used to select the register that will be modified and then sending the new
;; hardware colour value to set. It is important to remember that this commands
;; are processed by the PAL chip and it requires hardware colour values for INKR,
;; which are different from firmware colour values (which are used by BASIC's INK
;; command).
;;
;; Destroyed Register values:
;;    AF, BC, HL
;;
;; Required memory:
;;    16 bytes
;;
;; Time Measures:
;; (start code)
;;    Case    | Cycles | microSecs (us)
;; --------------------------------------
;;     Any    |  95    |  23.75
;; --------------------------------------
;; Asm saving | -41    | -10.25
;; --------------------------------------
;; (end code)
;;
;; Credits:
;;   This function has been constructed with great help from the
;; documentation at Grimware about the Gate Array.
;;
;; http://www.grimware.org/doku.php/documentations/devices/gatearray
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_setPALColour::
   ld    hl, #2            ;; [10] HL = SP + 2 (Place where parameters start) 
   add   hl, sp            ;; [11]
   ld     c, (hl)          ;; [ 7] C = First Parameter (PEN)
   inc   hl                ;; [ 6] 
   ld     a, (hl)          ;; [ 7] A = Second Parameter (INKR)

cpct_setPALColour_asm::    ;; Assembly entry point
  ;or  #PAL_PENR           ;; [ 7] (CCCnnnnn) Mix 3 bits for PENR command (C) and 5 for PEN number (n). As PENR command is 000, nothing to be done here.
   ld     b, #GA_port_byte ;; [ 7] B = Gate Array Port (0x7F). C has the command that GA will execute.
   out  (c), c             ;; [12] GA command: Select PENR. C = Command + Parameter (PENR + PEN number to select)

   or  #PAL_INKR           ;; [ 7] (CCCnnnnn) Mix 3 bits for INKR command (C) and 5 for INKR number (n). 
   out  (c), a             ;; [11] GA command: Set INKR. A = Command + Parameter (INKR + INK to be set for selected PEN number)

   ret                     ;; [10] Return