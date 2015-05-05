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
.module cpct_strings

;;
;; Include constants and general values
;;
.include /strings.s/

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_drawCharM0
;;
;;    Prints a ROM character on a given even-pixel position (byte-aligned) on the 
;; screen in Mode 0 (160x200 px, 16 colours).
;;
;; C Definition:
;;    void <cpct_drawCharM0> (void* *video_memory*, <u8> *fg_pen*, <u8> *bg_pen*, <i8> *ascii*)
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
;;    157 bytes (16 bytes conversion table, 141 bytes code)
;;
;; Time Measures:
;; (start code)
;;   Case     | Cycles | microSecs (us)
;; --------------------------------
;;   Best     |  3730  |   932.50
;;   Worst    |  4410  |  1102.50
;; --------------------------------
;; Asm saving |   -80  |   -20
;; --------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;
;; Colour table
;;
;; Bndry does not work when file is linked after being compiled.
;;.bndry 16   ;; Make this vector start at a 16-byte aligned address to be able to use 8-bit arithmetic with pointers
dc_mode0_ct:: .db 0x00, 0x40, 0x04, 0x44, 0x10, 0x50, 0x14, 0x54, 0x01, 0x41, 0x05, 0x45, 0x11, 0x51, 0x15, 0x55

_cpct_drawCharM0::
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

_cpct_drawCharM0_asm::
   ld  (dcm0_asciiHL+2), a     ;; [13] Save ASCII code of the character as data of a later "OR #data" instruction. 
                               ;; .... This is 1-cycle faster than pushing and popping to the stack and resets Carry Flag

   ;; Set up foreground and background colours for printing (getting them from tables)
   ;; -- Basically, we need to get values of the 4 bits that should be enabled when the a pixel is present
   ld   hl, #dc_mode0_ct       ;; [10] HL points to the start of the colour table

   ; This code only works if colour table is 16-byte aligned and saves up to 34 cycles
   ;LD  A, L                   ;; [ 4] HL += C (Foreground colour is an index in the colour table, so we increment HL by C bytes,
   ;ADD C                      ;; [ 4]          which makes HL point to the Foreground colour bits we need. This is valid because
   ;LD  L, A                   ;; [ 4]          table is 16-bytes aligned and we just need to increment L, as H won't change)
   ;SUB C                      ;; [ 4] A = L again (Make A save the original value of L, to use it again later with Background colour)
   ;LD  C, (HL)                ;; [ 7] C = Foreground colour bits
   ;ADD B                      ;; [ 4] HL += B (We increment HL with Background colour index, same as we did earlier with Foreground colour C)
   ;LD  L, A                   ;; [ 4]

   ld    a, l                  ;; [ 4] HL += C (Let HL point to the concrete colour in the table:
   add   c                     ;; [ 4]          HL points initial to the start of the table and C is the Foreground PEN number,
   ld    l, a                  ;; [ 4]          so HL+C is the memory location of the colour bits we need).
   adc   h                     ;; [ 4]
   sub   l                     ;; [ 4]
   ld    h, a                  ;; [ 4]

   ld    c, (hl)               ;; [ 7] C = Foreground colour bits
   ld   hl, #dc_mode0_ct       ;; [10] HL points again to the start of the colour table
   ld    a, l                  ;; [ 4] HL += B (Let HL point to the concrete colour in the table:
   add   b                     ;; [ 4]          HL points initial to the start of the table and B is the Background PEN number,
   ld    l, a                  ;; [ 4]          so HL+B is the memory location of the colour bits we need).
   adc   h                     ;; [ 4]
   sub   l                     ;; [ 4]
   ld    h, a                  ;; [ 4]

   ld    a, (hl)               ;; [ 7] A = Background colour bits
   ld (dcm0_drawForeground_0-2), a ;; [13] <| Modify Immediate value of "OR #0" to set it with the background colour bits
   ld (dcm0_drawForeground_1-2), a ;; [13] <|  (We do it 2 times, as 2 bits = 2 pixels = 1 byte in video memory)
   ld    a, c                      ;; [ 4] A = Foreground colour bits (Previously stored into C)
   ld (dcm0_drawForeground_0+1), a ;; [13] <| Modify Immediate value of "OR #0" to set it with the foreground colour bits
   ld (dcm0_drawForeground_1+1), a ;; [13] <|  (We do it 2 times too)

   ;; Calculate the memory address where the 8 bytes defining the character appearance 
   ;; ... start (HL = 0x3800 + 8*ASCII value). char0_ROM_address = 0x3800
dcm0_asciiHL:
   xor   a                     ;; [ 4] A = 0
   or    #0                    ;; [ 7] A = ASCII Value, and Resetting carry flag (#0 is a placeholder that will be filled up with ASII value)
   ld    h, #0x07              ;; [ 7] H = 0x07, because 0x07 * 8 = 0x38, (high byte of 0x3800)

   rla                         ;; [ 4] A = 8*A (using 3 rotates left). We use RLA as it passes exceeding bits 
   rl    h                     ;; [ 8] ... to the carry flag, and we can then pass them on to the 3 lowest bits of H
   rla                         ;; [ 4] ... using rl h. So H ends up being H = 8*H + A/32, what makes H be in the 
   rl    h                     ;; [ 8] ... [0x38-0x3F] range, where character definitions are.
   rla                         ;; [ 4]
   rl    h                     ;; [ 8] 

   ld    l, a                  ;; [ 4] L = A, so that HL points to the start of the character definition in ROM memory

   ;; Enable Lower ROM during char copy operation, with interrupts disabled 
   ;; to prevent firmware messing things up
   ld    a,(_cpct_mode_rom_status);; [13] A = mode_rom_status (present value)
   and   #0b11111011           ;; [ 7] bit 3 of A = 0 --> Lower ROM enabled (0 means enabled)
   ld    b, #GA_port_byte      ;; [ 7] B = Gate Array Port (0x7F)
   di                          ;; [ 4] Disable interrupts to prevent firmware from taking control while Lower ROM is enabled
   out (c), a                  ;; [12] GA Command: Set Video Mode and ROM status (100)

   ;; Do this for each pixel line (8-pixels)
dcm0_nextline:
   ld    c, (hl)               ;; [ 7] C = Next Character pixel line definition (8 bits defining 0 = background colour, 1 = foreground)
   ld    b, #4                 ;; [ 7] L = 4 bytes per line
   push de                     ;; [11] Save place where DE points now (start of the line) to be able to use it later to jump to next line

   ;; Do this for each video-memory-byte (2-pixels)
dcm0_next2pixels:
   xor   a                     ;; [ 4] A = 0 (A will hold the values of the next 2 pixels in video memory. They will be calculated as Character is read)

   ;; Transform first pixel
   sla   c                     ;; [ 8] Shift C (Char pixel line) left to know about Bit7 (next pixel) that will turn on / off the carry flag
   jp    c, dcm0_drawForeground_0 ;; [10] IF Carry, bit 7 was a 1, what means foreground colour
   or    #00                   ;; [ 7] Bit7 = 0, draw next pixel of Background colour
   .db #0xDA  ; JP C, xxxx     ;; [10] As carry is never set after an OR, this jump will never be done, and next 
                               ;; .... instruction will be 3 bytes from here (effectively jumping over OR xx without a jump) 
dcm0_drawForeground_0:
   or    #00                   ;; [ 7] Bit7 = 1, draw next pixel of Foreground colour
   rlca                        ;; [ 4] Rotate A 1 bit left to prepare it for inserting next pixel colour (same 2 bits
                               ;; .... will be operated but, as long as A is rotated first, we effectively operate on next 2 bits to the right)

   ;; Transform second pixel
   sla   c                     ;; [ 8] <\ Do the same as first pixel, for second one
   jp    c, dcm0_drawForeground_1 ;; [10]  |
   or    #00                   ;; [ 7]  |
   .db #0xDA  ; JP C, xxxx     ;; [10]  |
dcm0_drawForeground_1:         ;;       |
   or    #00                   ;; [ 7] </

   ld (de), a                  ;; [ 7] Save the 2 recently calculated pixels into video memory
   inc  de                     ;; [ 6] ... and point to next 2 pixels in video memory (next byte, DE++)

   djnz dcm0_next2pixels       ;; [13/8] If B != 0, continue with next 2 pixels of this pixel-line

dcm0_endpixelline:
   pop  de                     ;; [10] Recover previous DE position, to use it now for jumping to next video memory line

   ;; Move to next pixel-line definition of the character
   inc   l                     ;; [ 4] Next pixel Line (characters are 8-byte-aligned in memory, so we only need to increment L, as H will not change)
   ld    a, l                  ;; [ 4] IF next pixel line corresponds to a new character (this is, we have finished drawing our character),
   and   #0x07                 ;; [ 7] ... then L % 8 == 0, as it is 8-byte-aligned. 
   jp    z, dcm0_end_printing  ;; [10] IF L % 8 = 0, we have finished drawing the character, else, proceed to next line

   ;; Prepare to copy next line 
   ;;  -- Move DE pointer to the next pixel line on the video memory
   ld    a, d                  ;; [ 4] Start of next pixel line normally is 0x0800 bytes away.
   add   #0x08                 ;; [ 7]    so we add it to DE (just by adding 0x08 to D)
   ld    d, a                  ;; [ 4]
   and   #0x38                 ;; [ 7] We check if we have crossed memory boundary (every 8 pixel lines)
   jp   nz, dcm0_nextline      ;; [10]  by checking the 4 bits that identify present memory line. If 0, we have crossed boundaries
dcm0_8bit_boundary_crossed:
   ld    a, e                  ;; [ 4] DE = DE + 0xC050
   add   #0x50                 ;; [ 7]   -- Relocate DE pointer to the start of the next pixel line 
   ld    e, a                  ;; [ 4]   -- in video memory
   ld    a, d                  ;; [ 4]
   adc   #0xC0                 ;; [ 7]
   ld    d, a                  ;; [ 4]
   jp    dcm0_nextline         ;; [10] Jump to continue with next pixel line

dcm0_end_printing:
   ;; After finishing character printing, restore previous ROM and Interrupts status
   ld    a, (_cpct_mode_rom_status);; [13] A = mode_rom_status (present saved value)
   ld    b, #GA_port_byte      ;; [ 7] B = Gate Array Port (0x7F)
   out (c), a                  ;; [12] GA Command: Set Video Mode and ROM status (100)
   ei                          ;; [ 4] Enable interrupts

   ret                         ;; [10] Return
