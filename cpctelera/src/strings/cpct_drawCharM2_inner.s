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
;; Function: cpct_drawCharM2_inner_asm
;;
;;    Inner function used by <cpct_drawCharM2> and <cpct_drawStringM2> to actually
;; draw the character. This function shall not be used directly unless you know
;; exactly what it does.
;;
;; Input Parameters (3 Bytes):
;;  (2B HL) video_memory - Video memory location where the character will be drawn
;;  (1B A ) ascii        - Character to be drawn (ASCII code)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawCharM2_inner_asm
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
;;  * Screen is assumed to be standard mode 2 (640x480, 2 colours)
;;  * When reading from ROM, *interrupts should be disabled* to prevent firmware from
;; taking over and causing undefined behaviour. 
;;
;; Details:
;;    This function reads a character from ROM and draws it at a given *video_memory* location.
;; *video_memory* points to the upper-left corner of location where the character will be drawn. 
;; As this function assumes screen is configured for Mode 2 (640x480, 2 colours), it means that 
;; the character can only be drawn at pixel columns that are multple of 8 (0, 8, 8, 12...), 
;; because each byte contains 8 pixels in Mode 2. 
;;
;;    Character is drawn using 2 colours: foreground (FG) and background (BG). Both colours 
;; *must be* configured previously by calling <cpct_setDrawCharM2>. You may call this function 
;; once and then use the same colours for printing as long as you want. Every time you need 
;; different colours, you need to call <cpct_setDrawCharM2> again. Default colours are FG=1, BG=0
;; and will be used if <cpct_setDrawCharM2> has never been called previously.
;;
;;    This function is used by <cpct_drawCharM2> and <cpct_drawStringM2> as inner drawing loop.
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    38 bytes
;;
;; Time Measures:
;; (start code)
;;   Case       | microSecs | CPU Cycles 
;; ---------------------------------------
;;  FG/BG BF/FG |    145    |     580
;;  FG/FG BG/BG |    137    |     548
;; ---------------------------------------
;;   Worst      |    +10    |     +40
;; ---------------------------------------
;; (end code)  
;;    FG: Foreground, BG: Background
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Global address of cpct drawCharM2 inner modifiable code. Will be used by 
;; cpct_setdrawCharM2 to modify the code for drawing with different colours
cpct_charm2imc == nextrow

cpct_drawCharM2_inner_asm::
   ;; Calculate the memory address where the 8 bytes defining the character appearance 
   ;; ... start (DE = 0x3800 + 8*ASCII value). char0_ROM_address = 0x3800. 
   ;; ASCII value is in A=|hgfedcba|
   ld     d, #0x07   ;; [2] B = 0x07, because 0x07 * 8 = 0x38, (high byte of 0x3800)
   rla               ;; [1] A = 8*A (using 3 rotates left). We use RLA as it passes exceeding bits 
   rl     d          ;; [2] ... to the carry flag, and we can then pass them on to the 3 lowest bits of B
   rla               ;; [1] ... using rl b. So B ends up being B = 8*B + A/32, what makes H be in the 
   rl     d          ;; [2] ... [0x38-0x3F] range, where character definitions are.
   rla               ;; [1]
   rl     d          ;; [2] 
   ld     e, a       ;; [1] C = A, so that DE points to the start of the character definition in ROM memory
   ;; Now DE = |edcba|000||00111hgf| = 0x3800 + 8*ASCII

   ld     bc, #0x0808   ;; [3] B = Counter for the 8 lines of the character to be copied
                        ;;     C = Used later on to increment H by adding 8 after each pixel line
nextrow:
   ;; Next code gets modified by cpct_setDrawCharM2. When drawing video-inverted a CPL (0x2F) is inserted
   ;; between the two LDs that move the byte from DE to HL. When drawing all background or all foreground, 
   ;; a direct #00 or #FF insertion is performed
   ;; --------- Start of self-modifyiable code block
   ld    a, (de)  ;; [2] Copy 1 Character Line to Screen (DE -> HL)
   nop            ;; [1]  -- When painting in Foreground Colour, we do nothing
                  ;;      -- When painting Background Colour (inverted mode) this gets modified to a CPL (0x2F)
   ld (hl), a     ;; [2]
   ;; --------- End of self-modifyiable code block

   inc   e        ;; [1] ++E (Make DE Point to next character line defined at ROM)
                  ;;          Table is 8-byte aligned, so D never changes inside the definition of a character
   dec   b        ;; [1] --B (One less line of the character to be drawn)
   ret   z        ;; [2/4] IF B=0, end up printing, and return (all lines have been copied)

nextpixelline:
   ;; Prepare to copy next line 
   ;;  -- Move HL pointer to the next pixel line on the video memory
   ld     a, h       ;; [1] /
   add    c          ;; [1] | HL += 0x800 (Adding 8 to H as 00 is to be added to L)
   ld     h, a       ;; [1] \
   ;; Check if new address has crossed character boundaries (every 8 pixel lines)
   and   #0x38       ;; [2] We check if we have crossed memory boundary (every 8 pixel lines)
   jr    nz, nextrow ;; [2/3]  by checking the 4 bits that identify present memory line. 
                     ;; .... If 0, we have crossed boundaries
boundary_crossed:
   ld     a, #0x50   ;; [2] / 
   add    l          ;; [1] | HL = HL + 0xC050 
   ld     l, a       ;; [1] |  Relocate HL pointer to the start of the next pixel line in video memory
   ld     a, #0xC0   ;; [2] |
   adc    h          ;; [1] |
   ld     h, a       ;; [1] \
   jr    nextrow     ;; [3] Jump to continue with next pixel line
