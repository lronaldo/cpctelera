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
;; Function: cpct_drawCharM0
;;
;;    Draws a ROM character to the screen or hardware back-buffer in Mode 0 format
;; (160x200 px, 16 colours).
;;
;; C Definition:
;;    void <cpct_drawCharM0> (void* *video_memory*, <u8> *ascii*) __z88dk_callee
;;
;; Input Parameters (3 Bytes):
;;  (2B HL) video_memory - Video memory location where the character will be drawn
;;  (1B E ) ascii        - Character to be drawn (ASCII code)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawCharM0_asm
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
;;  * Screen must be configured in Mode 0 (160x200 px, 16 colours)
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
;;
;;    The character will be drawn using preconfigured foreground (FG) and 
;; background (BG) colours (by default BG=0, FG=1). Both colours can be set
;; calling <cpct_setDrawCharM0> before calling this function. Colours get set
;; by modifying and internal 4-byte array called _dc_2pxtableM0_. This means
;; that once colours are set they stay. You may set them once and call 
;; this function as many times as you want to draw with the same set of colours.
;;
;;    Next code example shows how to use this function in conjunction with
;; <cpct_setDrawCharM0>,
;; (start code)
;;    void drawSomeCharacters(u8* pscreen) {
;;       cpct_setDrawCharM0(5, 0);        // Foreground colour 5, Background 0
;;
;;       // Draw A, B, C consecutive
;;       cpct_drawCharM0(pscreen + 0, 'A');
;;       cpct_drawCharM0(pscreen + 4, 'B');
;;       cpct_drawCharM0(pscreen + 8, 'C');
;;
;;       // Set video inverse (inverted colours from before)
;;       cpct_setDrawCharM0(0, 5);        // Foreground colour 0, Background 5
;;
;;       // Draw D, E, F consecutive in inverse video
;;       cpct_drawCharM0(pscreen + 16, 'D');
;;       cpct_drawCharM0(pscreen + 20, 'E');
;;       cpct_drawCharM0(pscreen + 24, 'F');
;;    }
;; (end code)
;;
;; Destroyed Register values: 
;;    C bindings     - AF, BC, DE, HL
;;    ASM bindings   - AF, BC, DE, HL, IX
;;
;; Required memory:
;;    C bindings   - 35 bytes (+100 from cpct_drawCharM0_inner_asm = 135 bytes)
;;    ASM bindings - 23 bytes (+100 from cpct_drawCharM0_inner_asm = 123 bytes)
;;
;; Time Measures:
;; (start code)
;;   Case     | microSecs | CPU Cycles 
;; -------------------------------------
;;   Best     |    880    |    3520
;;   Worst    |    888    |    3552
;; -------------------------------------
;; Asm saving |    -25    |    -100
;; -------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.globl cpct_drawCharM0_inner_asm

   ;; Enable Lower ROM during char copy operation, with interrupts disabled 
   ;; to prevent firmware messing things up
   ld     a,(_cpct_mode_rom_status)  ;; [4] A = mode_rom_status (present value)
   and    #0b11111011                ;; [2] bit 3 of A = 0 --> Lower ROM enabled (0 means enabled)
   ld     b, #GA_port_byte           ;; [2] B = Gate Array Port (0x7F)
   di                                ;; [1] Disable interrupts to prevent firmware from taking control while Lower ROM is enabled
   out   (c), a                      ;; [3] GA Command: Set Video Mode and ROM status (100)

   ;; Draw the character
   ld     a, e                       ;; [1] A = ASCII Value of the character
   call   cpct_drawCharM0_inner_asm  ;; [828/837] Does the actual drawing to screen

endDraw:
   ;; After finishing character printing, restore previous ROM and Interrupts status
   ld     a, (_cpct_mode_rom_status) ;; [4] A = mode_rom_status (present saved value)
   ld     b, #GA_port_byte           ;; [2] B = Gate Array Port (0x7F)
   out   (c), a                      ;; [3] GA Command: Set Video Mode and ROM status (100)
   ei                                ;; [1] Enable interrupts

;; Restore IX and Return provided by bindings