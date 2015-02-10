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
;; Constant values
;;
.equ char0_ROM_address, 0x3800   ;; Address where definition of character 0 starts in ROM
.equ GA_port_byte,      0x7F     ;; 8-bit Port of the Gate Array


;;
;; External values
;;
.globl gfw_mode_rom_status       ;; defined in firmware_ed.s

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
;###     ted at whichever memory bank (0000h-FFFFh)                   ###
;###  -- This function disables interrupts during main loop (charac-  ###
;###     ter printing). It reenables them at the end.                 ###
;########################################################################
;### INPUTS (4 Bytes)                                                 ###
;###  * (2B DE) Video memory location where the char will be printed  ### 
;###  * (1B B) Foreground color (0 or 1, Background will be inverted) ###
;###  * (1B C) Character to be printed (ASCII code)                   ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURED TIME                                                    ###
;### MEMORY: 73 bytes                                                 ###
;### TIME:                                                            ###
;###  Best case  = 777 cycles (194.25 us)                             ###
;###  Worst case = 821 cycles (205.25 us)                             ###
;########################################################################
;

.globl _cpct_drawROMCharM2
_cpct_drawROMCharM2::
   ;; Get Parameters from stack (POP + Push)
   POP  AF                     ;; [10] AF = Return Address
   POP  DE                     ;; [10] DE = Pointer to video memory location where to print character
   POP  BC                     ;; [10] B = Character to be printed (ASCII), C = Foreground Color (0, 1)
   PUSH BC                     ;; [11] --- Restore Stack status ---
   PUSH DE                     ;; [11] 
   PUSH AF                     ;; [11]

   ;; Set up the color for printing, either foreground or video-inverted
   XOR A                       ;; [ 4] A = 00h (NOP machine code, which will do nothing in the main loop)
   LD  H, A                    ;; [ 4] H = 00h (We need it later to put ASCII code in HL, by making L = ASCII code)
   CP  C                       ;; [ 4] Check if C is 0 or not (C: Character color, 0=Inverted, 1=Foreground color)
   JP NZ, dc_print_fg_color    ;; [10] IF B != 0, then continue printing in normal foreground colour
   LD  A, #0x2F                ;; [ 7] A = 2Fh (CPL machine code, used to invert bytes in main loop)
dc_print_fg_color:
   LD (dc_nextline+1), A       ;; [13] Modify XOR code to be XOR #0xFF or XOR #0, to invert colours on printing or not

   ;; Make HL point to the starting byte of the desired character,
   ;; That is ==> HL = 8*(ASCII code) + char0_ROM_address
   LD   L, B                   ;; [ 4] HL = ASCII code of the character
   ;LD   H, #0                 ;; We did this before, with LD H, A

   ADD  HL, HL                 ;; [11] HL = HL * 8  (8 bytes each character)
   ADD  HL, HL                 ;; [11]
   ADD  HL, HL                 ;; [11]
   LD   BC, #char0_ROM_address ;; [10] BC = 0x3800, Start ROM memory address of Char 0
   ADD  HL, BC                 ;; [11] HL += BC

   LD  C, #8                   ;; [ 7] C = 8 lines counter (8 character lines to be printed out)

   ;; Enable Lower ROM during char copy operation, with interrupts disabled 
   ;; to prevent firmware messing things up
   LD   A,  (gfw_mode_rom_status) ;; [13] A = mode_rom_status (present value)
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
   ;; After finishing character printing, disable Lower ROM again and reenable interrupts
   LD   A,  (gfw_mode_rom_status) ;; [13] A = mode_rom_status (present value)
   OR   #0b00000100            ;; [ 7] bit 3 of A = 1 --> Lower ROM disabled (0 means enabled)
   ;LD   B,  #GA_port_byte     ;; [ 7] B = Gate Array Port (0x7F)
   OUT (C), A                  ;; [12] GA Command: Set Video Mode and ROM status (100)
   EI                          ;; [ 4] Enable interrupts

   RET                         ;; [10] Return

;
;########################################################################
;## FUNCTION: _cpct_drawCharM1                                        ###
;########################################################################
;### Draw char for mode 1                                             ###
;### Each character is 8 bytes long.                                  ###
;########################################################################
;### INPUTS (5 Bytes)                                                 ###
;###  * (2B DE) Video memory location where the char will be printed  ### 
;###  * (1B C) Foreground color (PEN, 0-3)                            ###
;###  * (1B B) Background color (PEN, 0-3)                            ###
;###  * (1B L) Character to be printed (ASCII code)                   ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURED TIME                                                    ###
;###                                                                  ###
;########################################################################
;

;;
;; Color tables
;;
.bndry 16   ;; Make this vector start at a 16-byte aligned address to be able to use 8-bit arithmetic with pointers
dc_mode0_ct: .db 0x00, 0x80, 0x08, 0x88, 0x20, 0xA0, 0x28, 0xA8, 0x02, 0x82, 0x0A, 0x8A, 0x22, 0xA2, 0x2A, 0xAA
.bndry 4    ;; Make this vector start at a 4-byte aligned address to be able to use 8-bit arithmetic with pointers
dc_mode1_ct: .db 0x00, 0x08, 0x80, 0x88

.globl _cpct_drawROMCharM1
_cpct_drawROMCharM1::
   ;; GET Parameters from the stack (Pop + Restoring SP)
   LD (dcm1_restoreSP+1), SP   ;; [20] Save SP into placeholder of the instruction LD SP, 0, to quickly restore it later.
   DI                          ;; [ 4] Disable interrupts to ensure no one overwrites return address in the stack
   POP  AF                     ;; [10] AF = Return Address
   POP  DE                     ;; [10] DE = Destination address (Video memory location where character will be printed)
   POP  BC                     ;; [10] BC = Colors (B=Background color, C=Foreground color) 
   POP  HL                     ;; [10] HL = ASCII code of the character (H=0, L=ASCII code)
dcm1_restoreSP:
   LD SP, #0                   ;; [10] -- Restore Stack Pointer -- (0 is a placeholder which is filled up with actual SP value previously)
   EI                          ;; [ 4] Enable interrupts again

   ;; [76] Set up foreground and background colours for printing (getting them from tables)
   ;; -- Basically, we need to get values of the 2 bits that should be enabled when the a pixel is present
   PUSH DE                     ;; [11] Save DE to be able to use it as temporal value for calculations

   LD   E, L                   ;; [ 4] E = L (Save ASCII code into E for later use)
   LD   D, B                   ;; [ 4] D = B (Save Background color PEN temporarily in D)
   LD   B, H                   ;; [ 4] B = 0 (Set B to 0 to be able to use BC as incrementer of HL: offset in the color table)
   LD  HL, #dc_mode1_ct        ;; [10] HL points to the start of the color table
   LD   A, L		       ;; [ 4] A = L (Save L into A, to restore HL later, as table is 16-byte-aligned and H will not be incremented)
   ADD HL, BC                  ;; [11] HL now points to the foreground color (HL + C), as each color is 1-byte sized (and C is foreground color PEN)
   LD   C, (HL)                ;; [ 7] C = Foreground color bits 
   ADD  D                      ;; [ 4] A += D (A contains previous value of L, and we increment it with previous value of B: Background color...
   LD   L, A                   ;; [ 4] ... and put it into L to make HL point to the Background color bits).
   LD   B, (HL)                ;; [ 7] B = Foreground color bits

   ;; Make HL point to the starting byte of the desired character,
   ;; That is ==> HL = 8*(ASCII code) + char0_ROM_address 
   LD   L, E                   ;; [ 4] L = E (Restore ASCII code into L)
   LD   H, #0                  ;; [ 7]

   ADD  HL, HL                 ;; [11] HL = HL * 8  (8 bytes each character)
   ADD  HL, HL                 ;; [11]
   ADD  HL, HL                 ;; [11]
   LD   DE, #char0_ROM_address ;; [10] DE = 0x3800, Start ROM memory address of Char 0
   ADD  HL, DE                 ;; [11] HL += DE

   POP  DE                     ;; [10] Restore value of DE previously saved

   LD  C, #8                   ;; [ 7] C = 8 lines counter (8 character lines to be printed out)

   ;; Enable Lower ROM during char copy operation, with interrupts disabled 
   ;; to prevent firmware messing things up
   LD   A,  (gfw_mode_rom_status) ;; [13] A = mode_rom_status (present value)
   AND  #0b11111011            ;; [ 7] bit 3 of A = 0 --> Lower ROM enabled (0 means enabled)
   LD   B,  #GA_port_byte      ;; [ 7] B = Gate Array Port (0x7F)
   DI                          ;; [ 4] Disable interrupts to prevent firmware from taking control while Lower ROM is enabled
   OUT (C), A                  ;; [12] GA Command: Set Video Mode and ROM status (100)

   ;; Copy character to Video Memory

   ;; Do this for each 4-pixels (1 byte)
dcm1_nextbyte:
   XOR A                       ;; [ 4] A = 0
   LD B, (HL)                  ;; [ 7] Copy 1 Character Line to Screen (HL -> DE)
   SLA B                       ;; [ 8] Shift B (char pixel line) left to know about next bit (pixel) that will turn on/off the carry flag
   JP C, dcm1_drawForeground   ;; [10] IF Carry, bit 7 was a 1, what means foreground color
   OR #0 ;colorfondo           ;; [ 7] Bit7=0, draw next pixel of Background color
   .db #0x38  ; JR C, xxxx     ;; [ 7] As carry is never set after an OR, this jump will never be done, and next instruction will be 3 bytes from here (effectively jumping over OR xx without a jump) 
dcm1_drawForeground:
   OR #0 ;colortexto           ;; [ 7] Bit7=1, draw next pixel of Foreground color

   LD (DE), A                  ;; [ 7]

   DEC C                       ;; [ 4] C-- (1 Character line less to finish)
   JP Z, dc_end_printing       ;; [10] IF C=0, end up printing (all lines have been copied)

   ;; Prepare to copy next line 
   ;;  -- Move DE pointer to the next pixel line on the video memory
   INC HL                      ;; [ 6] HL++ (Make HL point to next character line at ROM memory)

   LD  A, D                    ;; [ 4] Start of next pixel line normally is 0x0800 bytes away.
   ADD #0x08                   ;; [ 7]    so we add it to DE (just by adding 0x08 to D)
   LD  D, A                    ;; [ 4]
   AND #0x38                   ;; [ 7] We check if we have crossed memory boundary (every 8 pixel lines)
   JP NZ, dc_nextline          ;; [10]  by checking the 4 bits that identify present memory line. If 0, we have crossed boundaries
dcm1_8bit_boundary_crossed:
   LD  A, E                    ;; [ 4] DE = DE + C050h 
   ADD #0x50                   ;; [ 7]   -- Relocate DE pointer to the start of the next pixel line 
   LD  E, A                    ;; [ 4]   -- in video memory
   LD  A, D                    ;; [ 4]
   ADC #0xC0                   ;; [ 7]
   LD  D, A                    ;; [ 4]
   JP  dc_nextline             ;; [10] Jump to continue with next pixel line

dcm1_end_printing:
   ;; After finishing character printing, disable Lower ROM again and reenable interrupts
   LD   A,  (gfw_mode_rom_status) ;; [13] A = mode_rom_status (present value)
   OR   #0b00000100            ;; [ 7] bit 3 of A = 1 --> Lower ROM disabled (0 means enabled)
   ;LD   B,  #GA_port_byte     ;; [ 7] B = Gate Array Port (0x7F)
   OUT (C), A                  ;; [12] GA Command: Set Video Mode and ROM status (100)
   EI                          ;; [ 4] Enable interrupts

   RET                         ;; [10] Return

;   B < color transformed
;   LD A, color                 ;; []
;   RRA                         ;; [ 4]
;   JP NC, noset7               ;; [10]
;   SET 7, B                    ;; [ 8]
;noset7:
;   RRA                         ;; [ 4]
;   JP NC, noset3               ;; [10]
;   SET 3, B                    ;; [ 8]
;noset3:
