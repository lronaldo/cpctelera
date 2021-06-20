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

;;
;; Include constants and general values
;;
.include "macros/cpct_undocumentedOpcodes.h.s"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_drawCharM0_inner_asm
;;
;;    Inner function used by <cpct_drawCharM0> and <cpct_drawStringM0> to actually
;; draw the character. This function shall not be used directly unless you know
;; exactly what it does.
;;
;; Input Parameters (3 Bytes):
;;  (2B HL) video_memory - Video memory location where the character will be drawn
;;  (1B A ) ascii        - Character to be drawn (ASCII code)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawCharM0_inner_asm
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
;;  * To make this function *work from ROM*, put the 4 bytes of *dc_2pxtableM0* in RAM.
;;
;; Details:
;;    This function reads a character from ROM and draws it at a given *video_memory* location.
;; *video_memory* points to the upper-left corner of location where the character will be drawn. 
;; As this function assumes screen is configured for Mode 0 (160x200, 16 colours), it means that 
;; the character can only be drawn at even pixel columns (0, 2, 4, 8...), because each byte 
;; contains 2 pixels in Mode 0. 
;;
;;    Character is drawn using 2 colours: foreground (FG) and background (BG). Both colours 
;; *must be* configured previously by calling <cpct_setDrawCharM0>. You may call this function 
;; once and then use the same colours for printing as long as you want. Every time you need 
;; different colours, you need to call <cpct_setDrawCharM0> again. Default colours are FG=1, BG=0
;; and will be used if <cpct_setDrawCharM0> has never been called previously.
;;
;;    This function is used by <cpct_drawCharM0> and <cpct_drawStringM0> as inner drawing loop.
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL, IX
;;
;; Required memory:
;;    100 bytes
;;
;; Time Measures:
;; (start code)
;;   Case     | microSecs | CPU Cycles 
;; -------------------------------------
;;   Best     |    824    |    3300
;;   Worst    |    832    |    3332
;; -------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

cpct_drawCharM0_inner_asm::
   ;; Calculate the memory address where the 8 bytes defining the character appearance 
   ;; ... start (IX = 0x3800 + 8*ASCII value). char0_ROM_address = 0x3800
   rlca           ;; [1] | A = E = 8*ASCII. 3 RLCA leave A with this
   rlca           ;; [1] |   |hgfedcba| => 3*RLCA => |edcba|hgf|         IXH         IXL
   rlca           ;; [1] | Then we need to move it to IX like this => |00111hgf| |edcba000|
   ld    e, a     ;; [1] \ That will be the final memory address where the definition starts
   and   #0x07    ;; [2] Isolate latest 3 bits of a |00000hgf|
   or    #0x38    ;; [2] Add the 3 ones in front, so that the address starts at 0x38xx => |00111hgf|
   ld__ixh_a      ;; [2] Save it to IXH = |00111hgf|
   ld    a, e     ;; [1] Restore A status after 3*RLCA => |edcba|hgf|
   and   #0xF8    ;; [2] Isolate first 5 bits => |edcba|000|
   ld__ixl_a      ;; [2] and save it to IXL = |edcba|000|
   ;; Now IX = |edcba|000||00111hgf| = 0x3800 + 8*ASCII

   ld    bc, #dc_2pxtableM0    ;; [3] BC points to the 2 1-bit pixels to 2 4-bit pixels conversion table

   ;; Draw next line from the character to the screen
nextline:
   ex    de, hl      ;; [1] Put Destination pointer into DE (it is in HL)
   ld     a, (ix)    ;; [5] A = Next Character pixel line definition 
                     ;; .... (8 bits defining 0 = background colour, 1 = foreground)
   ;; Copy the 4 bytes that compose the complete pixel line
   ;; repeating the code for each pair of pixels to maximize speed
.rept 4
   ;; Convert next 2-bits into 1 byte with 2 pixels in screen pixel format
   ;; and copy it to (DE) which is next screen location
   ld    hl, #0      ;; [3] HL = 0
   rlca              ;; [1] /    Put the 2 leftmost bits of A into the two 
   rl    l           ;; [2] | ...rightmost bits of L. This is the combination for the
   rlca              ;; [1] | ...next 2 pixels (BG-BG, BG-FG, FG-BG, FG-FG). We use it
   rl    l           ;; [2] \ ...as index for the dc_2pxtableM0 which gets the actual pixel values.
   add   hl, bc      ;; [3] HL = BC + L (2pxtableM0 + Index = HL Points to the converted pixel values)
   ldi               ;; [5] Copy 2 pixels to the screen, incrementing DE at the same time
   inc   bc          ;; [2] BC is decremented by LDI but we want it to keep pointing to the table, so we add 1 again
.endm

endpixelline:
   ;; Move to next pixel-line definition of the character
   inc__ixl          ;; [2] Next pixel Line (characters are 8-byte-aligned in memory, 
                     ;; ... so we only need to increment IXL, as IXH will not change)
   ld__a_ixl         ;; [2] If next pixel line corresponds to a new character 
                     ;; .... (this is, we have finished drawing our character), ....
   and   #0x07       ;; [2] ... then L % 8 == 0, as it is 8-byte-aligned. 
   ret   z           ;; [2/4] If L % 8 == 0, we have finished drawing the character, else, proceed to next line

   ;; Prepare to copy next line 
   ;;  -- Move DE pointer to the next pixel line on the video memory
   ;; (We save new calculations on HL, because it will be exchanged with DE at the start of nextline: loop)
   ld    hl, #0x800-4      ;; [3] | Next pixel line is 0x800 bytes away in standard video modes
   add   hl, de            ;; [3] | ..but DE has already being incremented 4 times. So add 0x800-4 to
                           ;;       ..DE to make it point to the start of the next pixel line in video memory
   ;; Check if new address has crossed character boundaries (every 8 pixel lines)
   ld     a, h             ;; [1] A = H (top 8 bits of video memory address)
   and   #0x38             ;; [2] We check if we have crossed memory boundary (every 8 pixel lines)
   jr    nz, nextline      ;; [2/3]  by checking the 4 bits that identify present memory line. 
                           ;; .... If 0, we have crossed boundaries
boundary_crossed:
   ld    de, #0xC050       ;; [3] | HL = HL + 0xC050: Relocate DE pointer to the start of the next pixel line in video memory
   add   hl, de            ;; [3] \ (Remember that HL and DE will be exchanged at the start of nextline:)
   jr    nextline          ;; [3] Jump to continue with next pixel line

;; Conversion table from 2 1-bit pixels to mode 0 2 4-bit pixels. Essentially, there are 4
;; possible combinations with 2 pixels and 2 colours: (00, 01, 10, 11 == BG-BG, BG-FG, FG-BG, FG-FG)
;; We reserve here 4 bytes that will be filled in by <cpct_setDrawCharM0>
;;
dc_2pxtableM0:: .db 0x00, 0x40, 0x80, 0xC0   ;; Default colours BG=0, FG=1