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

;
;########################################################################
;## FUNCTION: _cpct_drawROMCharM2                                     ###
;########################################################################
;### This function reads a character from ROM and draws it at a given ###
;### point on the video memory (byte-aligned), assumming screen is    ###
;### configured for MODE 2. It can print the character in 2 different ###
;### color configurations:                                            ###
;###   Normal:   PEN 1 over PEN 0 (2nd Parameter [Color] > 0)         ###
;###   Inverted: PEN 0 over PEN 1 (2nd Parameter [Color] = 0)         ###
;### * Some IMPORTANT things to take into account:                    ###
;###  -- Do not put this function's code below 4000h in memory. In    ###
;###     order to read from ROM, this function enables Lower ROM      ###
;###     (which is located 0000h-3FFFh), so CPU would read from ROM   ###
;###     instead of RAM in first bank, effectively shadowing this     ###
;###     piece of code, and producing undefined results (tipically,   ###
;###     program would hang or crash).                                ###
;###  -- This function works well for drawing on double buffers loca- ###
;###     ted at whichever memory bank, except 0000h (4000h-FFFFh)     ###
;###  -- This function disables interrupts during main loop (charac-  ###
;###     ter printing). It reenables them at the end.                 ###
;########################################################################
;### INPUTS (4 Bytes)                                                 ###
;###  * (2B DE) Video memory location where the char will be printed  ### 
;###  * (1B C) Foreground color (0 or 1, Background will be inverted) ###
;###  * (1B B) Character to be printed (ASCII code)                   ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURES                                                         ###
;### MEMORY: 76 bytes                                                 ###
;### TIME:                                                            ###
;###  Best case  = 759 cycles (189.75 us)                             ###
;###  Worst case = 803 cycles (200.75 us)                             ###
;########################################################################
;

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
   ld   a, b                   ;; [ 4] A = ASCII Value
   ld   h, #0x07               ;; [ 7] H = 0x07, because 0x07 * 8 = 0x38, (high byte of 0x3800)

   rla                         ;; [ 4] A = 8*A (using 3 rotates left). We use RLA as it passes exceeding bits 
   rl   h                      ;; [ 8] ... to the carry flag, and we can then pass them on to the 3 lowest bits of H
   rla                         ;; [ 4] ... using rl h. So H ends up being H = 8*H + A/32, what makes H be in the 
   rl   h                      ;; [ 8] ... [0x38-0x3F] range, where character definitions are.
   rla                         ;; [ 4]
   rl   h                      ;; [ 8] 

   ld   l, a                   ;; [ 4] L = A, so that HL points to the start of the character definition in ROM memory

   LD  C, #8                   ;; [ 7] C = 8 lines counter (8 character lines to be printed out)

   ;; Enable Lower ROM during char copy operation, with interrupts disabled 
   ;; to prevent firmware messing things up
   LD   A,  (cpct_mode_rom_status) ;; [13] A = mode_rom_status (present value)
   AND  #0b11111011            ;; [ 7] bit 3 of A = 0 --> Lower ROM enabled (0 means enabled)
   LD   B,  #GA_port_byte      ;; [ 7] B = Gate Array Port (0x7F)
   DI                          ;; [ 4] Disable interrupts to prevent firmware from taking control while Lower ROM is enabled
   OUT (C), A                  ;; [12] GA Command: Set Video Mode and ROM status (100)

   ;; Copy character to Video Memory
dc_nextline:
   LD A, (HL)                  ;; [ 7] Copy 1 Character Line to Screen (HL -> DE)
   NOP                         ;; [ 4]  -- When we paint in Foreground Color, we do nothing, but this byte
                               ;;       -- gets modified NOP (00h) --> CPL (2Fh) to invert bits when painting Background Color (inverted mode)
   LD (DE), A                  ;; [ 7]

   DEC C                       ;; [ 4] C-- (1 Character line less to finish)
   JP Z, dc_end_printing       ;; [10] IF C=0, end up printing (all lines have been copied)

   ;; Prepare to copy next line 
   ;;  -- Move DE pointer to the next pixel line on the video memory
   INC L                       ;; [ 4] HL++ (Make HL point to next character line at ROM memory)
                               ;;      As characters are 8 bytes-long and table starts at 3800h, 
                               ;;      H never gets incremented on an INC HL, so we can use INC L instead (and save 3 cycles)

   LD  A, D                    ;; [ 4] Start of next pixel line normally is 0x0800 bytes away.
   ADD #0x08                   ;; [ 7]    so we add it to DE (just by adding 0x08 to D)
   LD  D, A                    ;; [ 4]
   AND #0x38                   ;; [ 7] We check if we have crossed memory boundary (every 8 pixel lines)
   JP NZ, dc_nextline          ;; [10]  by checking the 4 bits that identify present memory line. If 0, we have crossed boundaries
dc_8bit_boundary_crossed:
   LD  A, E                    ;; [ 4] DE = DE + C050h 
   ADD #0x50                   ;; [ 7]   -- Relocate DE pointer to the start of the next pixel line 
   LD  E, A                    ;; [ 4]   -- in video memory
   LD  A, D                    ;; [ 4]
   ADC #0xC0                   ;; [ 7]
   LD  D, A                    ;; [ 4]
   JP  dc_nextline             ;; [10] Jump to continue with next pixel line

dc_end_printing:
   ;; After finishing character printing, restore ROM and Interrupts status
   LD   A,  (cpct_mode_rom_status) ;; [13] A = mode_rom_status (present saved value)
   ;OR   #0b00000100            ;; [ 7] bit 3 of A = 1 --> Lower ROM disabled (0 means enabled)
   ;LD   B,  #GA_port_byte     ;; [ 7] B = Gate Array Port (0x7F)
   OUT (C), A                  ;; [12] GA Command: Set Video Mode and ROM status (100)
   EI                          ;; [ 4] Enable interrupts

   RET                         ;; [10] Return
