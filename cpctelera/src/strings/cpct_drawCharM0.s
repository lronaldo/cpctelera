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

_cpct_drawCharM0::
   ;; GET Parameters from the stack 
   pop   hl          ;; [3] HL = Return Address
   pop   de          ;; [3] DE = Pointer to Video Memory
   ex    (sp), hl    ;; [6] L = ASCII Value of the character to be drawn, and
                     ;; ...leaving Return Address on top of the stack at the same time
                     ;; ...as this function uses __z88dk_callee convention
   ld   (saveix), ix ;; [6] Save IX value before using it

cpct_drawCharM0_asm::
   ;; Calculate the memory address where the 8 bytes defining the character appearance 
   ;; ... start (IX = 0x3800 + 8*ASCII value). char0_ROM_address = 0x3800
   ld     a, l    ;; [1] A = ASCII Value of the character
   rlca           ;; [1] | L = 8*ASCII. 3 RLCA leave A with this
   rlca           ;; [1] |   |hgfedcba| => 3*RLCA => |edcba|hgf|         IXH         IXL
   rlca           ;; [1] | Then we need to move it to IX like this => |00111hgf| |edcba000|
   ld    l, a     ;; [1] \ That will be the final memory address where the definition starts
   and   #0x07    ;; [2] Isolate latest 3 bits of a |00000hgf|
   or    #0x38    ;; [2] Add the 3 ones in front, so that the address starts at 0x38xx => |00111hgf|
   ld__ixh_a      ;; [2] Save it to IXH = |00111hgf|
   ld    a, l     ;; [1] Restore A status after 3*RLCA => |edcba|hgf|
   and   #0xF8    ;; [2] Isolate first 5 bits => |edcba|000|
   ld__ixl_a      ;; [2] and save it to IXL = |edcba|000|
   ;; Now IX = |edcba|000||00111hgf| = 0x3800 + 8*ASCII

   ;; Enable Lower ROM during char copy operation, with interrupts disabled 
   ;; to prevent firmware messing things up
   ld     a,(_cpct_mode_rom_status) ;; [4] A = mode_rom_status (present value)
   and    #0b11111011               ;; [2] bit 3 of A = 0 --> Lower ROM enabled (0 means enabled)
   ld     b, #GA_port_byte          ;; [2] B = Gate Array Port (0x7F)
   di                               ;; [1] Disable interrupts to prevent firmware from taking control while Lower ROM is enabled
   out   (c), a                     ;; [3] GA Command: Set Video Mode and ROM status (100)

   ld    bc, #dc_2pxtableM0    ;; [3] BC points to the 2 1-bit pixels to 2 4-bit pixels conversion table

   ;; Draw next line from the character to the screen
nextline:
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
   jr    z, endDraw  ;; [2/3] If L % 8 == 0, we have finished drawing the character, else, proceed to next line

   ;; Prepare to copy next line 
   ;;  -- Move DE pointer to the next pixel line on the video memory
   ld    hl, #0x800-4      ;; [3] | Next pixel line is 0x800 bytes away in standard video modes
   add   hl, de            ;; [3] | ..but DE has already being incremented 4 times. So add 0x800-4 to
   ex    de, hl            ;; [1] \ ..DE to make it point to the start of the next pixel line in video memory
   ;; Check if new address has crossed character boundaries (every 8 pixel lines)
   ld     a, d             ;; [1] A = D (top 8 bits of video memory address)
   and   #0x38             ;; [2] We check if we have crossed memory boundary (every 8 pixel lines)
   jr    nz, nextline      ;; [2/3]  by checking the 4 bits that identify present memory line. 
                           ;; .... If 0, we have crossed boundaries
boundary_crossed:
   ld    hl, #0xC050       ;; [3] DE = DE + 0xC050
   add   hl, de            ;; [3]   -- Relocate DE pointer to the start of the next pixel line 
   ex    de, hl            ;; [1]   -- in video memory
   jr    nextline          ;; [3] Jump to continue with next pixel line

endDraw:
   ;; After finishing character printing, restore previous ROM and Interrupts status
   ld     a, (_cpct_mode_rom_status) ;; [4] A = mode_rom_status (present saved value)
   ld     b, #GA_port_byte           ;; [2] B = Gate Array Port (0x7F)
   out   (c), a                      ;; [3] GA Command: Set Video Mode and ROM status (100)
   ei                                ;; [1] Enable interrupts

saveix = .+2
   ld    ix, #0000   ;; [6] Restore IX before returning
   ret               ;; [3] Return

;; Conversion table from 2 1-bit pixels to mode 0 2 4-bit pixels. Essentially, there are 4
;; possible combinations with 2 pixels and 2 colours: (00, 01, 10, 11 == BG-BG, BG-FG, FG-BG, FG-FG)
;; We reserve here 4 bytes that will be filled in by <cpct_setDrawCharM0>
;;
dc_2pxtableM0:: .ds 4