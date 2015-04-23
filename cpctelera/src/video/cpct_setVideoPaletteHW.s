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
;; Function: cpct_setPalette
;;
;; Brief:
;;    Changes the hardware palette colour values (selecting new ones).
;;
;; C Definition:
;;    void *cpct_setPalette* (u8* *colour_array*, u8 *size*)
;;
;; Input Parameters (3 Bytes):
;;  (2B DE) colour_array  - Pointer to a byte array containing new hardware colour values [0 - 31]
;;  (1B A ) size          - [1 - 16] Number of colours to change
;;
;; Parameter Restrictions:
;;  * *colour_array* must be an array of unsigned 8-bit values (u8), each one
;; in the [0-31] range.
;;  * *size* must be between 1 and 16. A size of 0 will be treated as 16, and any
;; value greater than 16 will be modularized into [1-16] range (using only least
;; significant 4 bits)
;;
;; Requirements:
;;    This function requires the CPC *firmware* to be *DISABLED*. Otherwise, it
;; may not work, as firmware tends to restore palette colour values to its own
;; selection.
;;
;; Details:
;;    This function modifies hardware palette registers to set new colours to
;; use. These colours values are easily identified with PENs on the CPC, and are the ones
;; shown in the 2nd column of Table 1. The function starts modiying PEN 0, and goes 
;; changing PEN 1, 2, 3... up to the number of provided colours (*size*) from the 
;; *colour_array*. It is important to notice that this function requires
;; hardware colour values, which are different from firmware colour values (usually 
;; known as INKs on the CPC). Table 1 shows all of them, and you may use it for reference.
;;
;;    Palette registers are contained inside the PAL chip of the CPC (which is 
;; located inside the Gate Array). To access PAL chip, the same port (&7F) as Gate 
;; Array is used. The function is not thought to be optimal, but secure. If a more
;; optimal one is required, it can be derived from this code. The array of color 
;; values passed to this routine must include hardware color values (0-31) and not 
;; firmware ones.
;;
;; (start code)
;; Firmware | Hardware  | Hard+Command | Colour Name    |  R%   G%   B% Colour
;; ---------------------------------------------------------------------------
;;      0   | 20 (0x14) |  0x54        | Black          |   0    0    0
;;      1   |  4 (0x04) |  0x44 (0x50) | Blue           |   0    0   50
;;      2   | 21 (0x15) |  0x55        | Bright Blue    |   0    0  100
;;      3   | 28 (0x1C) |  0x5C        | Red            |  50    0    0
;;      4   | 24 (0x18) |  0x58        | Magenta        |  50    0   50
;;      5   | 29 (0x1D) |  0x5D        | Mauve          |  50    0  100
;;      6   | 12 (0x0C) |  0x4C        | Bright Red     | 100    0    0
;;      7   |  5 (0x05) |  0x45 (0x48) | Purple         | 100    0   50
;;      8   | 13 (0x0D) |  0x4D        | Bright Magenta | 100    0  100
;;      9   | 22 (0x16) |  0x56        | Green          |   0   50    0
;;     10   |  6 (0x06) |  0x46        | Cyan           |   0   50   50
;;     11   | 23 (0x17) |  0x57        | Sky Blue       |   0   50  100
;;     12   | 30 (0x1E) |  0x5E        | Yellow         |  50   50    0
;;     13   |  0 (0x00) |  0x40 (0x41) | White          |  50   50   50
;;     14   | 31 (0x1F) |  0x5F        | Pastel Blue    |  50   50  100
;;     15   | 14 (0x0E) |  0x4E        | Orange         | 100   50    0
;;     16   |  7 (0x07) |  0x47        | Pink           | 100   50   50
;;     17   | 15 (0x0F) |  0x4F        | Pastel Magenta | 100   50  100
;;     18   | 18 (0x12) |  0x52        | Bright Green   |   0  100    0
;;     19   |  2 (0x02) |  0x42 (0x51) | Sea Green      |   0  100   50
;;     20   | 19 (0x13) |  0x53        | Bright Cyan    |   0  100  100
;;     21   | 26 (0x1A) |  0x5A        | Lime           |  50  100    0
;;     22   | 25 (0x19) |  0x59        | Pastel Green   |  50  100   50
;;     23   | 27 (0x1B) |  0x5B        | Pastel Cyan    |  50  100  100
;;     24   | 10 (0x0A) |  0x4A        | Bright Yellow  | 100  100    0
;;     25   |  3 (0x03) |  0x43 (0x49) | Pastel Yellow  | 100  100   50
;;     26   | 11 (0x0B) |  0x4B        | Bright White   | 100  100  100
;; ---------------------------------------------------------------------------
;;    Table 1. Colour values on the CPC (Firmware, Hardware and RGB)
;; (end)
;;
;;    Table 1 shows Firmware colour values (column 1), Hardware correspondant colour
;; values (column 2) and classic Hardware values (column 3) which include the colour
;; value (5 Least significant bits) along with the command for the Gate Array (3 Most
;; significant bits). As this function already provides hardware commands, the values
;; it requires are the ones from column 2 (Hardware). Table 1 also provides colour 
;; names and precentages of RGB values for better understanding.
;; 
;; Known limitations: 
;;    This function cannot set blinking colors, as blinking colours are managed 
;; by the firmware (not by hardware).
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL 
;;
;; Required memory:
;;    35 bytes
;;
;; Time Measures:
;; (start code)
;; Case  | Cycles      | microSecs (us)
;; ---------------------------------------
;; Any   | 101 + 78*NC | 25,25 + 19,50*NC
;; (end)
;;    NC=Number of colours to be set
;;
;;  Example - On setting NC=16 colours: 1349 cycles (337.25 us)
;;
;; Credits:
;;    This function has been constructed with great help from the
;; documentation at Grimware about the Gate Array.
;;
;; http://www.grimware.org/doku.php/documentations/devices/gatearray
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_setPalette::
   ;; Getting parameters from stack
   ld  hl, #2               ;; [10] HL = SP + 2 (Place where parameters start) 
   add hl, sp               ;; [11]
   ld   e, (hl)             ;; [ 7] DE = First parameter, pointer to the array that contains desired color IDs 
   inc  hl                  ;; [ 6]      (should get it in little endian order, E, then D)
   ld   d, (hl)             ;; [ 7]
   inc  hl                  ;; [ 6]
   ld   a, (hl)             ;; [ 7] A = Second parameter, Number of colours to set (up to 16) 

   ex   de, hl              ;; [ 4] HL = DE, We will use HL to point to the array and get colour ID values
   dec  a                   ;; [ 4] A -= 1 (We leave A as 1 colour less to convert 16 into 15 and be able to use AND to ensure no more than 16 colours are passed)
   and  #0x0F               ;; [ 7] A %= 16, A will be 15 at most, that is, the number of colours to set minus 1
   inc  a                   ;; [ 4] A += 1 Restore the 1 to leave A as the number of colours to set
   ld   e, a                ;; [ 4] E = A, E will have the number of colours to set 
   ld   d, #0               ;; [ 7] D = 0, D will count from 0 to E 

   ld   b, #GA_port_byte    ;; [ 7] BC = Gate Array Port (0x7F), to send commands using OUT
svp_setPens_loop:
   ld   a, d                ;; [ 4] A = D, A has index of the next PEN to be established
  ;OR   #PAL_PENR           ;; [ 7] A = (CCCnnnnn) Mix 3 bits for PENR command (C) and 5 for PEN number (n). As PENR command is 000, nothing to be done here.
   out (c),a                ;; [11] GA Command: Select PENR. A = Command + Parameter (PENR + PEN number to select)

   ld   a, (hl)             ;; [ 7] Get Color value (INK) for the selected PEN from array
   and  #0x1F               ;; [ 7] Leave out only the 5 Least significant bits (3 MSB can cause problems)
   or   #PAL_INKR           ;; [ 7] (CCCnnnnn) Mix 3 bits for INKR command (C) and 5 for PEN number (n). 
   out (c),a                ;; [11] GA Command: Set INKR. A = Command + Parameter (INKR + INK to be set for selected PEN number)

   inc  hl                  ;; [ 6] HL += 1, Point to the next INK in the array
   inc  d                   ;; [ 4] D += 1, Next PEN index to be set 
   dec  e                   ;; [ 4] E -= 1, count how many PENs still to be set
   jp nz, svp_setPens_loop  ;; [10] If more than 0 PENs to be set, continue

   ret                      ;; [10] Return
