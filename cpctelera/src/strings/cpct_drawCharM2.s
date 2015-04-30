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
;; Function: cpct_drawCharM2
;;
;;    Prints a ROM character on a given byte-aligned position on the screen 
;; in Mode 2 (640x200 px, 2 colours).
;;
;; C Definition:
;;    void <cpct_drawCharM2> (void* *video_memory*, <u8> *pen*, <i8> *ascii*)
;;
;; Input Parameters (4 Bytes):
;;  (2B DE) video_memory - Video memory location where the character will be drawn
;;  (1B C ) pen          - Colour configuration (!=0 Normal / =0 Inverted)
;;  (1B B ) ascii        - Character to be printed (ASCII code)
;;
;; Parameter Restrictions:
;;  * *video_memory* could theoretically be any 16-bit memory location. It will work
;; outside current screen memory boundaries, which is useful if you use any kind of
;; double buffer. However, be careful where you use it, as it does no kind of check
;; or clipping, and it could overwrite data if you select a wrong place to draw.
;;  * *pen* 0 = Inverted, >0 = Normal. Normal means foreground colour = PEN 1, 
;; background colour = PEN 0. Inverted means the contrary of normal. 
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
;;
;; Details:
;;    This function reads a character from ROM and draws it at a given byte-aligned 
;; video memory location, that corresponds to the upper-left corner of the 
;; character. As this function assumes screen is configured for Mode 2
;; (640x200, 2 colours), it means that the character can only be drawn at module-8 
;; pixel columns (0, 8, 16, 24...), because each byte contains 8 pixels in Mode 2.
;; It can print the character in 2 different colour configurations:
;;  Normal   - PEN 1 over PEN 0 (*pen* > 0)
;;  Inverted - PEN 0 over PEN 1 (*pen* = 0)
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    76 bytes
;;
;; Time Measures:
;; (start code)
;; Case   | Cycles | microSecs (us)
;; --------------------------------
;; Best   |   759  |   189.75
;; Worst  |   803  |   200.75
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_drawCharM2::
   ;; Get Parameters from stack (POP + Push)
   pop  af                     ;; [10] AF = Return Address
   pop  de                     ;; [10] DE = Pointer to video memory location where to print character
   pop  bc                     ;; [10] B = Character to be printed (ASCII), C = Foreground Color (0, 1)
   push bc                     ;; [11] --- Restore Stack status ---
   push de                     ;; [11] 
   push af                     ;; [11]

_cpct_drawCharM2_asm::

   ;; Set up the color for printing, either foreground or video-inverted
   xor   a                     ;; [ 4] A = 0 + Carry reset (NOP machine code, which will do nothing in the main loop)
   dec   c                     ;; [ 4] Check if C is 0 or not, preserving Carry Flag status (C: Character color, 0=Inverted, 1=Foreground color)
   jp    p, dc_print_fg_color  ;; [10] IF B != 0 (C is still positive after dec), then continue printing in normal foreground colour
   ld    a, #0x2F              ;; [ 7] A = 2Fh (CPL machine code, used to invert bytes in main loop)
dc_print_fg_color:
   ld (dc_nextline+1), a       ;; [13] Modify XOR code to be XOR #0xFF or XOR #0, to invert colours on printing or not

   ;; Calculate the memory address where the 8 bytes defining the character appearance 
   ;; ... start (HL = 0x3800 + 8*ASCII value). char0_ROM_address = 0x3800
   ld    a, b                  ;; [ 4] A = ASCII Value
   ld    h, #0x07              ;; [ 7] H = 0x07, because 0x07 * 8 = 0x38, (high byte of 0x3800)

   rla                         ;; [ 4] A = 8*A (using 3 rotates left). We use RLA as it passes exceeding bits 
   rl    h                     ;; [ 8] ... to the carry flag, and we can then pass them on to the 3 lowest bits of H
   rla                         ;; [ 4] ... using rl h. So H ends up being H = 8*H + A/32, what makes H be in the 
   rl    h                     ;; [ 8] ... [0x38-0x3F] range, where character definitions are.
   rla                         ;; [ 4]
   rl    h                     ;; [ 8] 

   ld    l, a                  ;; [ 4] L = A, so that HL points to the start of the character definition in ROM memory

   ld    c, #8                 ;; [ 7] C = 8 lines counter (8 character lines to be printed out)

   ;; Enable Lower ROM during char copy operation, with interrupts disabled 
   ;; to prevent firmware messing things up
   ld    a, (_cpct_mode_rom_status) ;; [13] A = mode_rom_status (present value)
   and   #0b11111011           ;; [ 7] bit 3 of A = 0 --> Lower ROM enabled (0 means enabled)
   ld    b, #GA_port_byte      ;; [ 7] B = Gate Array Port (0x7F)
   di                          ;; [ 4] Disable interrupts to prevent firmware from taking control while Lower ROM is enabled
   out (c), a                  ;; [12] GA Command: Set Video Mode and ROM status (100)

   ;; Copy character to Video Memory
dc_nextline:
   ld    a, (hl)               ;; [ 7] Copy 1 Character Line to Screen (HL -> DE)
   nop                         ;; [ 4]  -- When we paint in Foreground Color, we do nothing, but this byte
                               ;;       -- gets modified NOP (00h) --> CPL (2Fh) to invert bits when painting Background Color (inverted mode)
   ld (de), a                  ;; [ 7]

   dec   c                     ;; [ 4] C-- (1 Character line less to finish)
   jp    z, dc_end_printing    ;; [10] IF C=0, end up printing (all lines have been copied)

   ;; Prepare to copy next line 
   ;;  -- Move DE pointer to the next pixel line on the video memory
   inc   l                     ;; [ 4] HL++ (Make HL point to next character line at ROM memory)
                               ;;      As characters are 8 bytes-long and table starts at 3800h, 
                               ;;      H never gets incremented on an INC HL, so we can use INC L instead (and save 3 cycles)

   ld    a, d                  ;; [ 4] Start of next pixel line normally is 0x0800 bytes away.
   add   #0x08                 ;; [ 7]    so we add it to DE (just by adding 0x08 to D)
   ld    d, a                  ;; [ 4]
   and   #0x38                 ;; [ 7] We check if we have crossed memory boundary (every 8 pixel lines)
   jp   nz, dc_nextline        ;; [10]  by checking the 4 bits that identify present memory line. If 0, we have crossed boundaries
dc_8bit_boundary_crossed:
   ld    a, e                  ;; [ 4] DE = DE + C050h 
   add   #0x50                 ;; [ 7]   -- Relocate DE pointer to the start of the next pixel line 
   ld    e, a                  ;; [ 4]   -- in video memory
   ld    a, d                  ;; [ 4]
   adc   #0xC0                 ;; [ 7]
   ld    d, a                  ;; [ 4]
   jp    dc_nextline           ;; [10] Jump to continue with next pixel line

dc_end_printing:
   ;; After finishing character printing, restore ROM and Interrupts status
   ld    a, (_cpct_mode_rom_status) ;; [13] A = mode_rom_status (present saved value)
   ;OR   #0b00000100            ;; [ 7] bit 3 of A = 1 --> Lower ROM disabled (0 means enabled)
   ;LD   B,  #GA_port_byte     ;; [ 7] B = Gate Array Port (0x7F)
   out (c), a                  ;; [12] GA Command: Set Video Mode and ROM status (100)
   ei                          ;; [ 4] Enable interrupts

   ret                         ;; [10] Return
