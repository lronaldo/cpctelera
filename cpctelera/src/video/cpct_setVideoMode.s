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
;; Function: cpct_setVideoMode
;;
;;    Sets the video mode of the CPC Screen, changing resolution and palette size.
;;
;; C Definition:
;;    void <cpct_setVideoMode> (<u8> *videoMode*)
;;
;; Input Parameters (1 Byte):
;;    (1B A) videoMode - [0-3] Video mode to set
;;
;; Parameter Restrictions: 
;;  - *videomode* must be < 3, otherwise unexpected results may happen. Namely,
;; ROM/RAM pagination and Interrupt Status may get altered, typically yielding
;; erratic behaviour and/or crashes.
;;
;; Requirements:
;;    This function requires the CPC *firmware* to be *DISABLED*. Otherwise, it
;; may not work, as firmware tends to restore video mode to its own selection.
;;
;; Details:
;;    This function changes the video mode for the Amstrad CPC into one of the
;; 4 standard modes available. This 4 standard video modes are:
;; (start code)
;; Mode | Resolution | # of colours
;; -----|------------|-------------------
;;  0   |  160x200   |   16
;;  1   |  320x200   |    4
;;  2   |  640x200   |    2
;;  3   |  160x200   |    4 (undocumented)
;; (end)
;;    The way this function works is by sending a SET_VIDEO_MODE command directly
;; to the Gate Array (GA), through the 0x7F port. The 2 least significant bits from
;; the byte sent to the GA contain the desired videomode.
;;
;; Destroyed Register values:
;;    AF, BC, HL
;;
;; Required memory:
;;    18 bytes
;;
;; Time Measures:
;; (start code)
;; Case  | Cycles | microSecs (us)
;; -------------------------------
;; Any   |  71    |  17.75
;; (end code)
;;
;; Credits:
;;    This function was coded copying and modifying cpc_setMode from
;; cpcrslib by Raul Simarro.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_setVideoMode::
   ;; Get Parameter from stack
   ld   hl, #2               ;; [10] HL = SP + 2 (Place where parameters are in the stack)
   add  hl, sp               ;; [11]
   ld    c, (hl)             ;; [ 7] A = First Paramter (Video Mode to be selected)

   ld   hl, #cpct_mode_rom_status ;; [10] HL points to present MODE, INT.GEN and ROM selection byte.
   ld    a, (hl)             ;; [ 7] A = Present values for MODE, INT.GEN and ROM selection. (See mode_rom_status)
   and #0xFC                 ;; [ 7] A = (xxxxxx00) set bits 1,0 to 0, to prepare them for inserting the mode parameter
   or    c                   ;; [ 4] A = Mixes previously selected ING.GEN and ROM values with user selected MODE (last 2 bits)
   ld    b, #GA_port_byte    ;; [ 7] B = Gate Array Port (0x7F)
   out (c), a                ;; [12] GA Command: Set Video Mode

   ld (hl), a                ;; [ 7] Save new Mode and ROM status for later use if required

   ret                       ;; [10] Return
