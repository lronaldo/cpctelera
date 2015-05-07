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
.module cpct_video
  
.include /videomode.s/

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Function: cpct_getHWColour
;;
;;    Converts a firmware colour value into its equivalent hardware one.
;;
;; C Definition:
;;    <u8> <cpct_getHWColour> (<u8> *firmware_colour*)
;;
;; Input Parameters (1 Bytes):
;;    (1B BC) firmware_colour - [0-26] Firmware colour value to be converted (Similar to BASIC's INK value)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_getHWColour_asm
;;    * BC = *firmware_colour* implies that B = 0 and C = *firmware_colour*
;;
;; Parameter Restrictions:
;;    * *firmware_colour* must be in the range [0-26], otherwise, return value will be unexpected.
;;
;; Return Value:
;;    <u8> - [0-31] Hardware colour value corresponding to *firmware_colour* provided.
;;
;; Details:
;;    Uses the *firmware_colour* as index in a conversion table to get its equivalent hardware
;; value. You can find this equivalences in the table 1 of the <cpct_setPalette> explanation.
;;
;; Destroyed Register values:
;;    BC, HL
;;
;; Required memory:
;;    40 bytes (13 bytes code, 27 bytes colour conversion table)
;;
;; Time Measures:
;; (start code)
;;     Case   | Cycles  | microSecs (us)
;; ---------------------------------------
;;     Any    |   63    |   15.75
;; ---------------------------------------
;; Asm saving |  -32    |   -8.00
;; ---------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_getHWColour::
   ld   hl, #2               ;; [10] HL = SP + 2 (Place where parameters start) 
   ld    b, h                ;; [ 4] B = 0, to be able to use C as an incremente for HL (adding BC)
   add  hl, sp               ;; [11]
   ld    c, (hl)             ;; [ 7] A = Firmware INK colour value 

cpct_getHWColour_asm::       ;; Assembly entry point

   ld   hl, #cpct_firmware2hw_colour ;; [10] HL points to the start of the colour table
   add  hl, bc               ;; [11] HL += C (as B=0), HL points to the exact hardware color value to return

   ld    l, (hl)             ;; [ 7] L = Return value (hardware colour for firmware colour supplied)
   ld    h, b                ;; [ 4] H = 0, to leave HL just with the value of L

   ret                      ;; [10] Return
