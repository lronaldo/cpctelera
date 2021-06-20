;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2018 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_drawCharM1
;;
;;    Prints a ROM character on a given byte-aligned position on the screen 
;; in Mode 1 (320x200 px, 4 colours).
;;
;; C Definition:
;;    void <cpct_drawCharM1> (void* *video_memory*, <u8> *ascii*) __z88dk_callee
;;
;; Input Parameters (3 Bytes):
;;  (2B HL) video_memory - Video memory location where the character will be drawn
;;  (1B E ) ascii        - Character to be drawn (ASCII code)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawCharM1_asm
;;
;; Parameter Restrictions:
;;  * *video_memory* could theoretically be any 16-bit memory location. It will work
;; outside current screen memory boundaries, which is useful if you use any kind of
;; double buffer. However, be careful where you use it, as it does no kind of check
;; or clipping, and it could overwrite data if you select a wrong place to draw.
;;  * *fg_pen* must be in the range [0-3]. It is used to access a colour mask table and,
;; so, a value greater than 3 will return a random colour mask giving unpredictable 
;; results (typically bad character rendering, with odd colour bars).
;;  * *bg_pen* must be in the range [0-3], with identical reasons to *fg_pen*.
;;  * *ascii* could be any 8-bit value, as 256 characters are available in ROM.
;;
;; Requirements and limitations:
;;  * *Do not put this function's code below 0x4000 in memory*. In order to read
;; characters from ROM, this function enables Lower ROM (which is located 0x0000-0x3FFF),
;; so CPU would read code from ROM instead of RAM in first bank, effectively shadowing
;; this piece of code. This would lead to undefined results (typically program would
;; hang or crash).
;;  * Screen must be configured in Mode 1 (320x200 px, 4 colours)
;;  * This function requires the CPC *firmware* to be *DISABLED*. Otherwise, random
;; crashes might happen due to side effects.
;;  * This function *disables interrupts* during main loop (character printing), and
;; re-enables them at the end.
;;  * This function *will not work from ROM*, as it uses self-modifying code.
;;
;; Details:
;;    This function reads a character from ROM and draws it at a given byte-aligned 
;; video memory location, that corresponds to the upper-left corner of the 
;; character. As this function assumes screen is configured for Mode 1
;; (320x200, 4 colours), it means that the character can only be drawn at module-4 
;; pixel columns (0, 4, 8, 12...), because each byte contains 4 pixels in Mode 1.
;; It prints the character in 2 colours (PENs) one for foreground (*fg_pen*), and 
;; the other for background (*bg_pen*). 
;;
;;    The character will be drawn using preconfigured foreground (FG) and 
;; background (BG) colours (by default BG=0, FG=1). Both colours can be set
;; calling <cpct_setDrawCharM1> before calling this function. Colours get set
;; by modifying and internal 16-byte array called _cpct_char2pxM1_. This means
;; that once colours are set they stay. You may set them once and call 
;; this function as many times as you want to draw with the same set of colours.
;;
;;    Next code example shows how to use this function in conjunction with
;; <cpct_setDrawCharM1>,
;; (start code)
;;    void drawSomeCharactersM1(u8* pscreen, u8 fgcolour, u8 bgcolour) {
;;       cpct_setDrawCharM1(fgcolour, bgcolour);   // Set colours before drawing
;;
;;       // Draw A, B, C consecutive
;;       cpct_drawCharM1(pscreen +  0, 'A');
;;       cpct_drawCharM1(pscreen +  2, 'B');
;;       cpct_drawCharM1(pscreen +  4, 'C');
;;
;;       // Set video inverse (inverted colours from before)
;;       cpct_setDrawCharM1(bgcolour, fgcolour);   // Set inverse colours
;;
;;       // Draw D, E, F consecutive in inverse video
;;       cpct_drawCharM1(pscreen +  6, 'D');
;;       cpct_drawCharM1(pscreen +  8, 'E');
;;       cpct_drawCharM1(pscreen + 10, 'F');
;;    }
;; (end code)
;;
;; Destroyed Register values: 
;;    C bindings     - AF, BC, DE, HL
;;    ASM bindings   - AF, BC, DE, HL, IX
;;
;; Required memory:
;;    C bindings     - 35 bytes (+80 bytes cpct_drawCharM1_inner_asm = 115 bytes)
;;    ASM bindings   - 23 bytes (+80 bytes cpct_drawCharM1_inner_asm = 103 bytes)
;;
;; Time Measures:
;; (start code)
;;   Case     | microSecs (us) | CPU Cycles
;; -------------------------------------------
;;    Best    |       514      |     2056
;;    Worst   |       522      |     2088
;; -------------------------------------------
;; Asm saving |       -28      |     -112
;; -------------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.globl cpct_drawCharM1_inner_asm

   ;; Enable Lower ROM during char copy operation, with interrupts disabled 
   ;; to prevent firmware messing things up
   ld     a,(_cpct_mode_rom_status)  ;; [4] A = mode_rom_status (present value)
   and    #0b11111011                ;; [2] bit 3 of A = 0 --> Lower ROM enabled (0 means enabled)
   ld     b, #GA_port_byte           ;; [2] B = Gate Array Port (0x7F)
   di                                ;; [1] Disable interrupts to prevent firmware from taking control while Lower ROM is enabled
   out   (c), a                      ;; [3] GA Command: Set Video Mode and ROM status (100)

   ;; Draw the character
   ld     a, e                       ;; [1] A = ASCII Value of the character
   call   cpct_drawCharM1_inner_asm  ;; [5+458/466] Does the actual drawing to screen

endDraw:
   ;; After finishing character printing, restore previous ROM and Interrupts status
   ld     a, (_cpct_mode_rom_status) ;; [4] A = mode_rom_status (present saved value)
   ld     b, #GA_port_byte           ;; [2] B = Gate Array Port (0x7F)
   out   (c), a                      ;; [3] GA Command: Set Video Mode and ROM status (100)
   ei                                ;; [1] Enable interrupts

;; Restore IX and Return provided by bindings