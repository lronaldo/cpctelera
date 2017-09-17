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
;#####################################################################
;### MODULE: Strings                                               ###
;#####################################################################
;### Routines to print and manage characters and strings           ###
;#####################################################################
;
.module cpct_strings

;;
;; Include constants and general values
;;
.include /strings.s/

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_drawCharM1
;;
;;    Prints a ROM character on a given byte-aligned position on the screen 
;; in Mode 1 (320x200 px, 4 colours).
;;
;; C Definition:
;;    void <cpct_drawCharM1> (void* *video_memory*, <u8> *fg_pen*, <u8> *bg_pen*, <u8> *ascii*)
;;
;; Input Parameters (5 Bytes):
;;  (2B DE) video_memory - Video memory location where the character will be drawn
;;  (1B C )  fg_pen       - Foreground palette colour index (Similar to BASIC's PEN, 0-3)
;;  (1B B )  bg_pen       - Background palette colour index (PEN, 0-3)
;;  (1B A )  ascii        - Character to be drawn (ASCII code)
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
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    139 bytes (4 bytes conversion table, 135 bytes code)
;;
;; Time Measures:
;; (start code)
;;   Case     | Cycles | microSecs (us)
;; ------------------------------------
;;   Best     |  4378  |  1094.50
;;   Worst    |  5058  |  1264.50
;; ------------------------------------
;; Asm saving |   -80  |   -20
;; ------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;
;; Color table
;;
.globl dc_mode1_ct

_cpct_drawCharM1::
   ;; GET Parameters from the stack 
   ld    hl, #2                ;; [10] HL = Pointer to the place where parameters start (SP + 2)
   add   hl, sp                ;; [11]
   ld     e, (hl)              ;; [ 7] DE = Destination address (Video memory location where character will be printed) 
   inc   hl                    ;; [ 6]
   ld     d, (hl)              ;; [ 7]
   inc   hl                    ;; [ 6]
   ld     c, (hl)              ;; [ 7] C = Foreground colour
   inc   hl                    ;; [ 6]
   ld     b, (hl)              ;; [ 7] B = Background colour
   inc   hl                    ;; [ 6]
   ld     a, (hl)              ;; [ 7] A = ASCII code of the character

cpct_drawCharM1_asm::

   ;; Set up foreground and background colours for printing (getting them from tables)
   ;; -- Basically, we need to get values of the 2 bits that should be enabled when the a pixel is present
   ld  (dcm1_asciiHL+2), a     ;; [13] Save ASCII code of the character as data of a later "OR #data" instruction. 
                               ;; .... This is 1-cycle faster than pushing and popping to the stack and resets Carry Flag

   ld  hl, #dc_mode1_ct        ;; [10] HL points to the start of the color table
   ;LD  A, L                   ;; [ 4] HL += C (Foreground color is an index in the color table, so we increment 
   ;ADD C                      ;; [ 4]          HL by C bytes, which makes HL point to the Foreground color bits we need. 
   ;LD  L, A                   ;; [ 4]          This is valid because table is 4-bytes aligned and we just need 
                               ;; ....          to increment L, as H won't change)
   ;SUB C                      ;; [ 4] A = L again (Make A save the original value of L, to use it again 
   ;                           ;; ....              later with Background color)
   ;LD  C, (HL)                ;; [ 7] C = Foreground color bits
   ;ADD B                      ;; [ 4] HL += B (We increment HL with Background color index, 
   ;LD  L, A                   ;; [ 4]          same as we did earlier with Foreground color C)

   ld  a, l                    ;; [ 4] HL += C (Let HL point to the concrete color in the table:
   add c                       ;; [ 4]          HL points initial to the start of the table and C is 
   ld  l, a                    ;; [ 4]          the Foreground PEN number, so HL+C is the memory location of 
   adc h                       ;; [ 4]          the color bits we need).
   sub l                       ;; [ 4]
   ld  h, a                    ;; [ 4]

   ld  c, (hl)                 ;; [ 7] C = Foreground color bits
   ld  hl, #dc_mode1_ct        ;; [10] HL points again to the start of the color table
   ld  a, l                    ;; [ 4] HL += B (Let HL point to the concrete color in the table:
   add b                       ;; [ 4]          HL points initial to the start of the table and B is 
   ld  l, a                    ;; [ 4]          the Background PEN number, so HL+B is the memory location of 
   adc h                       ;; [ 4]          the color bits we need).
   sub l                       ;; [ 4]
   ld  h, a                    ;; [ 4]

   ld  a, (hl)                 ;; [ 7] A = Background color bits
   ld (dcm1_drawForeground-2), a ;; [13] Modify Inmediate value of "OR #0" to set it with the background color bits
   ld  a, c                    ;; [ 4] A = Foreground color bits (Previously stored into C)
   ld (dcm1_drawForeground+1), a ;; [13] Modify Inmediate value of "OR #0" to set it with the foreground color bits

   ;; Calculate the memory address where the 8 bytes defining the character appearance 
   ;; ... start (HL = 0x3800 + 8*ASCII value). char0_ROM_address = 0x3800
dcm1_asciiHL:
   xor  a                      ;; [ 4] A = 0
   or   #0                     ;; [ 7] A = ASCII Value and Resetting carry flag 
                               ;; ....     (#0 is a placeholder that will be filled up with ASII value)
   ld   h, #0x07               ;; [ 7] H = 0x07, because 0x07 * 8 = 0x38, (high byte of 0x3800)

   rla                         ;; [ 4] A = 8*A (using 3 rotates left). We use RLA as it passes exceeding bits 
   rl   h                      ;; [ 8] ... to the carry flag, and we can then pass them on to the 3 lowest bits of H
   rla                         ;; [ 4] ... using rl h. So H ends up being H = 8*H + A/32, what makes H be in the 
   rl   h                      ;; [ 8] ... [0x38-0x3F] range, where character definitions are.
   rla                         ;; [ 4]
   rl   h                      ;; [ 8] 

   ld   l, a                   ;; [ 4] L = A, so that HL points to the start of the character definition in ROM memory
   
   ;; Enable Lower ROM during char copy operation, with interrupts disabled 
   ;; to prevent firmware messing things up
   ld   a,(_cpct_mode_rom_status);; [13] A = mode_rom_status (present value)
   and  #0b11111011            ;; [ 7] bit 3 of A = 0 --> Lower ROM enabled (0 means enabled)
   ld   b, #GA_port_byte       ;; [ 7] B = Gate Array Port (0x7F)
   di                          ;; [ 4] Disable interrupts to prevent firmware from taking control while Lower ROM is enabled
   out (c), a                  ;; [12] GA Command: Set Video Mode and ROM status (100)

   ;; Do this each pixel line (8-pixels)
dcm1_nextline:
   ld c, (hl)                  ;; [ 7] C = Next Character pixel line definition 
                               ;; .... (8 bits defining 0=background color, 1=foreground)
   push hl                     ;; [11] Save HL register to be able to use it as temporal storage
   ld l, #2                    ;; [ 7] L=2 bytes per line

   ;; Do this each video-memory-byte (4-pixels)
dcm1_next4pixels:
   xor a                       ;; [ 4] A = 0 (A will hold the values of the next 4 pixels in video memory. 
                               ;; ....        They will be calculated as Character is read)
   ld  b, #4                   ;; [ 7] B = 4 (4 pixels for each byte)

   ;; Do this each pixel inside a byte (each byte has 4 pixels) 
dcm1_nextpixel:
   sla c                       ;; [ 8] Shift C (Char pixel line) left to know about Bit7 (next pixel) 
                               ;; .... that will turn on/off the carry flag
   jp c, dcm1_drawForeground   ;; [10] IF Carry, bit 7 was a 1, what means foreground color
   or  #00                     ;; [ 7] Bit7=0, draw next pixel of Background color
   .db #0xDA  ; JP C, xxxx     ;; [10] As carry is never set after an OR, this jump will never be done, and next 
                               ;; .... instruction will be 3 bytes from here (effectively jumping over OR xx without a jump) 
dcm1_drawForeground:
   or  #00                     ;; [ 7] Bit7=1, draw next pixel of Foreground color
   rlca                        ;; [ 4] Rotate A 1 bit left to prepare it for inserting next pixel color 
                               ;; .... (same 2 bits will be operated but, as long as A is rotated first,
                               ;; ....  we effectively operate on next 2 bits to the right)
   djnz dcm1_nextpixel         ;; [13/8] IF B!=0, continue calculations with next pixel

   ld (de), a                  ;; [ 7] Save the 4 recently calculated pixels into video memory

   dec l                       ;; [ 4] L-- (1 byte less to finish this line)
   jp z, dcm1_endpixelline     ;; [10] If L=0, we have finished the line

   inc de                      ;; [ 6] ... and point to next 4 pixels in video memory (next byte, DE++)

   jp dcm1_next4pixels         ;; [10] Continue with next 4 pixels

dcm1_endpixelline:
   ;; Move to next pixel-line definition of the character
   pop hl                      ;; [10] Restore HL previous value from stack
   inc l                       ;; [ 4] Next pixel Line (characters are 8-byte-aligned in memory, 
                               ;; .... so we only need to increment L, as H will not change)
   ld  a, l                    ;; [ 4] IF next pixel line corresponds to a new character 
                               ;; .... (this is, we have finished drawing our character), ....
   and #0x07                   ;; [ 7] ... then L % 8 == 0, as it is 8-byte-aligned. 
   jp z, dcm1_end_printing     ;; [10] IF L%8 = 0, we have finished drawing the character, else, proceed to next line

   ;; Prepare to copy next line 
   ;;  -- Move DE pointer to the next pixel line on the video memory
   dec de                      ;; [ 6] DE-- : Restore DE to previous position in memory (it has moved 1 byte forward)
   ld  a, d                    ;; [ 4] Start of next pixel line normally is 0x0800 bytes away.
   add #0x08                   ;; [ 7]    so we add it to DE (just by adding 0x08 to D)
   ld  d, a                    ;; [ 4]
   and #0x38                   ;; [ 7] We check if we have crossed memory boundary (every 8 pixel lines)
   jp nz, dcm1_nextline        ;; [10]  by checking the 4 bits that identify present memory line.
                               ;; ....  If 0, we have crossed boundaries
dcm1_8bit_boundary_crossed:
   ld  a, e                    ;; [ 4] DE = DE + C050h 
   add #0x50                   ;; [ 7]   -- Relocate DE pointer to the start of the next pixel line 
   ld  e, a                    ;; [ 4]   -- in video memory
   ld  a, d                    ;; [ 4]
   adc #0xC0                   ;; [ 7]
   ld  d, a                    ;; [ 4]
   jp  dcm1_nextline           ;; [10] Jump to continue with next pixel line

dcm1_end_printing:
   ;; After finishing character printing, restore previous ROM and Interrupts status
   ld   a,(_cpct_mode_rom_status);; [13] A = mode_rom_status (present saved value)
   ld   b,  #GA_port_byte      ;; [ 7] B = Gate Array Port (0x7F)
   out (c), a                  ;; [12] GA Command: Set Video Mode and ROM status (100)
   ei                          ;; [ 4] Enable interrupts

   ret                         ;; [10] Return
