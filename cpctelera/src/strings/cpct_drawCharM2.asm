;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
;;
;;  This program is free software: you can redistribute it and/or modify
;;  it under the terms of the GNU Lesser General Public License as published by
;;  the Free Software Foundation, either version 3 of the License, or
;;  (at your option) any later version.
;;
;;  This program is distributed in the hope that it will be useful,
;;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;  GNU Lesser General Public License for more details.
;;
;;  You should have received a copy of the GNU Lesser General Public License
;;  along with this program.  If not, see <http://www.gnu.org/licenses/>.
;;-------------------------------------------------------------------------------

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_drawCharM2
;;
;;    Prints a ROM character on a given byte-aligned position on the screen 
;; in Mode 2 (640x200 px, 2 colours).
;;
;; C Definition:
;;    void <cpct_drawCharM2> (void* *video_memory*, <u8> *ascii*)
;;
;; Input Parameters (3 Bytes):
;;  (2B HL) video_memory - Video memory location where the character will be drawn
;;  (1B E ) ascii        - Character to be printed (ASCII code)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawCharM2_asm
;;
;; Parameter Restrictions:
;;  * *video_memory* could theoretically be any 16-bit memory location. It will work
;; outside current screen memory boundaries, which is useful if you use any kind of
;; double buffer. However, be careful where you use it, as it does no kind of check
;; or clipping, and it could overwrite data if you select a wrong place to draw.
;;  * *ascii* could be any 8-bit value, as 256 characters are available in ROM.
;;
;; Requirements and limitations:
;;  * *Do not put this function's code below 0x4000 in memory*. In order to read
;; characters from ROM, this function enables Lower ROM (which is located 0x0000-0x3FFF),
;; so CPU would read code from ROM instead of RAM in first bank, effectively shadowing
;; this piece of code. This would lead to undefined results (typically program would
;; hang or crash).
;;  * Screen must be configured in Mode 2 (640x200 px, 2 colours)
;;  * This function requires the CPC *firmware* to be *DISABLED*. Otherwise, random
;; crashes might happen due to side effects.
;;  * This function *disables interrupts* during main loop (character printing), and
;; re-enables them at the end.
;;  * This function *enables low ROM* during main loop (character printing), and
;; disables it again at the end.
;;  * This function *will not work from ROM*, as it uses self-modifying code.
;;
;; Details:
;;    This function reads a character from ROM and draws it at a given byte-aligned 
;; video memory location, that corresponds to the upper-left corner of the 
;; character. As this function assumes screen is configured for Mode 2
;; (640x200, 2 colours), it means that the character can only be drawn at module-8 
;; pixel columns (0, 8, 16, 24...), because each byte contains 8 pixels in Mode 2.
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    C-bindings  - 27 bytes (+38 bytes from <cpct_drawCharM2_inner_asm> = 65 bytes)
;;  ASM-bindings  - 23 bytes (+38 bytes from <cpct_drawCharM2_inner_asm> = 61 bytes)
;;
;; Time Measures:
;; (start code)
;;   Case     | microSecs | CPU Cycles
;; ------------------------------------
;;   Best     |    181    |    624
;;   Worst    |    199    |    796
;; ------------------------------------
;; Asm saving |    -13    |    -52
;; ------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.globl cpct_drawCharM2_inner_asm

   ;; Enable Lower ROM during char copy operation, with interrupts disabled 
   ;; to prevent firmware messing things up
   ld     a,(_cpct_mode_rom_status)  ;; [4] A = mode_rom_status (present value)
   and    #0b11111011                ;; [2] bit 3 of A = 0 --> Lower ROM enabled (0 means enabled)
   ld     b, #GA_port_byte           ;; [2] B = Gate Array Port (0x7F)
   di                                ;; [1] Disable interrupts to prevent firmware from taking control while Lower ROM is enabled
   out   (c), a                      ;; [3] GA Command: Set Video Mode and ROM status (100)

   ;; Draw the character
   ld     a, e                       ;; [1] A = ASCII Value of the character
   call   cpct_drawCharM2_inner_asm  ;; [5+137/155] Does the actual drawing to screen

endDraw:
   ;; After finishing character printing, restore previous ROM and Interrupts status
   ld     a, (_cpct_mode_rom_status) ;; [4] A = mode_rom_status (present saved value)
   ld     b, #GA_port_byte           ;; [2] B = Gate Array Port (0x7F)
   out   (c), a                      ;; [3] GA Command: Set Video Mode and ROM status (100)
   ei                                ;; [1] Enable interrupts

   ret                               ;; [3] Return to caller