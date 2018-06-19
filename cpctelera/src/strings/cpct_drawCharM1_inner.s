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
;; Function: cpct_drawCharM1_inner_asm
;;
;;    Inner function used by <cpct_drawCharM1> and <cpct_drawStringM1> to actually
;; draw the character. This function shall not be used directly unless you know
;; exactly what it does.
;;
;; Input Parameters (3 Bytes):
;;  (2B HL) video_memory - Video memory location where the character will be drawn
;;  (1B A ) ascii        - Character to be drawn (ASCII code)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawCharM1_inner_asm
;;
;; Parameter Restrictions:
;;  * *video_memory* could theoretically be any 16-bit memory location. It will work
;; outside current screen memory boundaries, which is useful if you use any kind of
;; double buffer. However, be careful where you use it, as it does no kind of check
;; or clipping, and it could overwrite data if you select a wrong place to draw.
;;  * *ascii* could be any 8-bit value, as 256 characters are available in ROM.
;;
;; Requirements and limitations:
;;  * This function *assumes Lower ROM (0x0000-0x3FFF) is enabled* to read ROM character 
;; definitions. Therefore, this code should be above 0x3FFF to work; otherwise, it would
;; become shadowed by ROM and results would be undefined. You may, theoretically, use it
;; without ROM enabled and using your own character set definitions in RAM (0x3800-0x3FFF)
;;  * Screen is assumed to be standard mode 0 (160x200, 16 colours)
;;  * When reading from ROM, *interrupts should be disabled* to prevent firmware from
;; taking over and causing undefined behaviour. 
;;  * To make this function *work from ROM*, put the 16 bytes of *cpct_char2pxM1* in RAM.
;;
;; Details:
;;    This function reads a character from ROM and draws it at a given *video_memory* location.
;; *video_memory* points to the upper-left corner of location where the character will be drawn. 
;; As this function assumes screen is configured for Mode 1 (320x200, 4 colours), it means that 
;; the character can only be drawn at even pixel columns (0, 4, 8, 12...), because each byte 
;; contains 4 pixels in Mode 1. 
;;
;;    Character is drawn using 2 colours: foreground (FG) and background (BG). Both colours 
;; *must be* configured previously by calling <cpct_setDrawCharM1>. You may call this function 
;; once and then use the same colours for printing as long as you want. Every time you need 
;; different colours, you need to call <cpct_setDrawCharM1> again. Default colours are FG=1, BG=0
;; and will be used if <cpct_setDrawCharM1> has never been called previously.
;;
;;    This function is used by <cpct_drawCharM1> and <cpct_drawStringM1> as inner drawing loop.
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    80 bytes (64 bytes code + 16 bytes colour table)
;;
;; Time Measures:
;; (start code)
;;   Case     | microSecs | CPU Cycles 
;; -------------------------------------
;;   Best     |    458    |    1832
;;   Worst    |    466    |    1848
;; -------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

cpct_drawCharM1_inner_asm::
   ;; Calculate the memory address where the 8 bytes defining the character appearance 
   ;; ... start (BC = 0x3800 + 8*ASCII value). char0_ROM_address = 0x3800. 
   ;; ASCII value is in A=|hgfedcba|
   ld     b, #0x07    ;; [2] B = 0x07, because 0x07 * 8 = 0x38, (high byte of 0x3800)
   rla                ;; [1] A = 8*A (using 3 rotates left). We use RLA as it passes exceeding bits 
   rl     b           ;; [2] ... to the carry flag, and we can then pass them on to the 3 lowest bits of B
   rla                ;; [1] ... using rl b. So B ends up being B = 8*B + A/32, what makes H be in the 
   rl     b           ;; [2] ... [0x38-0x3F] range, where character definitions are.
   rla                ;; [1]
   rl     b           ;; [2] 
   ld     c, a        ;; [1] C = A, so that BC points to the start of the character definition in ROM memory
   ;; Now BC = |edcba|000||00111hgf| = 0x3800 + 8*ASCII

nextrow:
   ;; HL holds destination video memory address where to draw next
   ;; Lets put it on DE and use HL to point to the conversion table
   ex    de, hl       ;; [1] DE points to video memory, HL is free

   ;; Draw first 4 pixels (1st byte) of the row to the screen
   ld    hl, #char2px ;; [3] HL points to char2pixels conversion table
   ld     a, (bc)     ;; [2] Get current row definition to extract the high nibble, which defines first 4 pixels
   rrca               ;; [1] / Switch both nibbles of A. We want to use the high nibble (4 highest bits)
   rrca               ;; [1] | as a value to be added to the base address of the char2px table (now in HL)
   rrca               ;; [1] | to find the actual conversion to pixels.
   rrca               ;; [1] \ A = |abcdefgh| >>> A' = |efghabcd| 
   and  #0x0F         ;; [2] A'' = |0000abcd| (Leave only the 4 highest bits of A as a 0-15 number)
   add    l           ;; [1] /
   ld     l, a        ;; [1] | HL' = HL + A  
   adc    h           ;; [1] | We add the highest nibble of A to HL to get the first 4 pixel values to be
   sub    l           ;; [1] | drawn to the screen (the first of two bytes to be written)
   ld     h, a        ;; [1] \
   ld     a, (hl)     ;; [2] / Write first 4 pixels to the screen and increment destination pointer to leave
   ld  (de), a        ;; [2] | it ready for the next 4 pixels.
   inc   de           ;; [2] \ (DE) <- (HL) : DE++

   ;; Draw second 4 pixels (2nd byte) of the row to the screen
   ld    hl, #char2px ;; [3] HL points to char2pixels conversion table again
   ld     a, (bc)     ;; [2] Get current row definition again, but this time to extract low nibble, defining next 4 pixels
   and  #0x0F         ;; [2] A = |abcdefgh| >>> A' = |0000efgh| (Leave only lowest nibble as a 0-15 value)
   add    l           ;; [1] / 
   ld     l, a        ;; [1] | HL' = HL + A  
   adc    h           ;; [1] | We add the lowest nibble of A to HL to get the next 4 pixel values to be
   sub    l           ;; [1] | drawn to the screen (the second of two bytes to be written)
   ld     h, a        ;; [1] \
   ld     a, (hl)     ;; [2] / Write next 4 pixels to the screen 
   ld  (de), a        ;; [2] \ (DE) <- (HL)

endpixelline:
   ;; Move to next pixel-line definition of the character
   inc    c           ;; [1] Next pixel Line (characters are 8-byte-aligned in memory, 
                      ;; ... so we only need to increment C, as B will not change)
   ld     a, c        ;; [1] If next pixel line corresponds to a new character 
                      ;; .... (this is, we have finished drawing our character), ....
   and   #0x07        ;; [2] ... then C % 8 == 0, as it is 8-byte-aligned. 
   ret   z            ;; [2/4] If C % 8 == 0, we have finished drawing the character, else, proceed to next line

   ;; Prepare to copy next line 
   ;;  -- Move DE pointer to the next pixel line on the video memory
   ;; (We save new calculations on HL, because it will be exchanged with DE at the start of nextrow: loop)
   ld    hl, #0x800-1 ;; [3] | Next pixel line is 0x800 bytes away in standard video modes
   add   hl, de       ;; [3] | ..but DE has already being incremented by 1. So add 0x800-1 to
                      ;;       ..DE to make it point to the start of the next pixel line in video memory
   ;; Check if new address has crossed character boundaries (every 8 pixel lines)
   ld     a, h        ;; [1] A = H (top 8 bits of video memory address)
   and   #0x38        ;; [2] We check if we have crossed memory boundary (every 8 pixel lines)
   jr    nz, nextrow  ;; [2/3]  by checking the 4 bits that identify present memory line. 
                      ;; .... If 0, we have crossed boundaries
boundary_crossed:
   ld    de, #0xC050  ;; [3] | HL = HL + 0xC050: Relocate DE pointer to the start of the next pixel line in video memory
   add   hl, de       ;; [3] \ (Remember that HL and DE will be exchanged at the start of nextrow:)
   jr    nextrow      ;; [3] Jump to continue with next pixel line

;; Character To Pixels Definition conversion table.
;; This table is set up with the 16 combinations for pixel values using the current 
;; PEN/PAPER selected configuration. This is used to convert the character definition
;; to actual pixel values and then render them to screen
cpct_char2pxM1:: .ds 16
char2px = cpct_char2pxM1   ;; Alias for brevity
