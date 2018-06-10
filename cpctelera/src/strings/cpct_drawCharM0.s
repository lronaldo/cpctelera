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
.module cpct_strings

;;
;; Include constants and general values
;;
.include "strings.s"
.include "macros/cpct_undocumentedOpcodes.h.s"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_drawCharM0
;;
;;    Prints a ROM character on a given even-pixel position (byte-aligned) on the 
;; screen in Mode 0 (160x200 px, 16 colours).
;;
;; C Definition:
;;    void <cpct_drawCharM0> (void* *video_memory*, <u8> *fg_pen*, <u8> *bg_pen*, <u8> *ascii*)
;;
;; Input Parameters (5 Bytes):
;;  (2B DE) video_memory - Video memory location where the character will be drawn
;;  (1B C )  fg_pen       - Foreground palette colour index (Similar to BASIC's PEN, 0-15)
;;  (1B B )  bg_pen       - Background palette colour index (PEN, 0-15)
;;  (1B A )  ascii        - Character to be drawn (ASCII code)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawCharM0_asm
;;
;; Parameter Restrictions:
;;  * *video_memory* could theoretically be any 16-bit memory location. It will work
;; outside current screen memory boundaries, which is useful if you use any kind of
;; double buffer. However, be careful where you use it, as it does no kind of check
;; or clipping, and it could overwrite data if you select a wrong place to draw.
;;  * *fg_pen* must be in the range [0-15]. It is used to access a colour mask table and,
;; so, a value greater than 15 will return a random colour mask giving unpredictable 
;; results (typically bad character rendering, with odd colour bars).
;;  * *bg_pen* must be in the range [0-15], with identical reasons to *fg_pen*.
;;  * *ascii* could be any 8-bit value, as 256 characters are available in ROM.
;;
;; Requirements and limitations:
;;  * *Do not put this function's code below 0x4000 in memory*. In order to read
;; characters from ROM, this function enables Lower ROM (which is located 0x0000-0x3FFF),
;; so CPU would read code from ROM instead of RAM in first bank, effectively shadowing
;; this piece of code. This would lead to undefined results (typically program would
;; hang or crash).
;;  * Screen must be configured in Mode 0 (160x200 px, 16 colours)
;;  * This function requires the CPC *firmware* to be *DISABLED*. Otherwise, random
;; crashes might happen due to side effects.
;;  * This function *disables interrupts* during main loop (character printing), and
;; re-enables them at the end.
;;  * This function *will not work from ROM*, as it uses self-modifying code.
;;
;; Details:
;;    This function reads a character from ROM and draws it at a given byte-aligned 
;; video memory location, that corresponds to the upper-left corner of the 
;; character. As this function assumes screen is configured for Mode 0
;; (160x200, 16 colours), it means that the character can only be drawn at even 
;; pixel columns (0, 2, 4, 8...), because each byte contains 2 pixels in Mode 0.
;; It prints the character in 2 colours (PENs) one for foreground (*fg_pen*), and 
;; the other for background (*bg_pen*). 
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    133 bytes 
;;
;; Time Measures:
;; (start code)
;;   Case     | microSecs | CPU Cycles 
;; -------------------------------------
;;   Best     |    885    |    3540
;;   Worst    |    894    |    3576
;; -------------------------------------
;; Asm saving |    -18    |     -76
;; -------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.globl cpct_drawCharM0_inner_asm

_cpct_drawCharM0::
   ;; GET Parameters from the stack 
   pop   hl          ;; [3] HL = Return Address
   pop   de          ;; [3] DE = Pointer to Video Memory
   ex    (sp), hl    ;; [6] L = ASCII Value of the character to be drawn, and
                     ;; ...leaving Return Address on top of the stack at the same time
                     ;; ...as this function uses __z88dk_callee convention
   ld   (saveix), ix ;; [6] Save IX value before using it

cpct_drawCharM0_asm::
   ;; Enable Lower ROM during char copy operation, with interrupts disabled 
   ;; to prevent firmware messing things up
   ld     a,(_cpct_mode_rom_status) ;; [4] A = mode_rom_status (present value)
   and    #0b11111011               ;; [2] bit 3 of A = 0 --> Lower ROM enabled (0 means enabled)
   ld     b, #GA_port_byte          ;; [2] B = Gate Array Port (0x7F)
   di                               ;; [1] Disable interrupts to prevent firmware from taking control while Lower ROM is enabled
   out   (c), a                     ;; [3] GA Command: Set Video Mode and ROM status (100)

   ;; Draw the character
   ld     a, l                      ;; [1] A = ASCII Value of the character
   call   cpct_drawCharM0_inner_asm ;; [824/833] Does the actual drawing to screen

endDraw:
   ;; After finishing character printing, restore previous ROM and Interrupts status
   ld     a, (_cpct_mode_rom_status) ;; [4] A = mode_rom_status (present saved value)
   ld     b, #GA_port_byte           ;; [2] B = Gate Array Port (0x7F)
   out   (c), a                      ;; [3] GA Command: Set Video Mode and ROM status (100)
   ei                                ;; [1] Enable interrupts

saveix = .+2
   ld    ix, #0000   ;; [6] Restore IX before returning
   ret               ;; [3] Return
