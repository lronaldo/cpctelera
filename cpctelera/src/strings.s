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
;### MEASURES                                                         ###
;### MEMORY: 71 bytes                                                 ###
;### TIME:                                                            ###
;###  Best case  = 770 cycles (192.50 us)                             ###
;###  Worst case = 814 cycles (203.50 us)                             ###
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
   ;; After finishing character printing, restore ROM and Interrupts status
   LD   A,  (gfw_mode_rom_status) ;; [13] A = mode_rom_status (present saved value)
   ;OR   #0b00000100            ;; [ 7] bit 3 of A = 1 --> Lower ROM disabled (0 means enabled)
   ;LD   B,  #GA_port_byte     ;; [ 7] B = Gate Array Port (0x7F)
   OUT (C), A                  ;; [12] GA Command: Set Video Mode and ROM status (100)
   EI                          ;; [ 4] Enable interrupts

   RET                         ;; [10] Return

;
;########################################################################
;## FUNCTION: _cpct_drawROMCharM1                                     ###
;########################################################################
;### This function reads a character from ROM and draws it at a given ###
;### point on the video memory (byte-aligned), assumming screen is    ###
;### configured for MODE 1. It prints the character in 2 colors(PENs) ###
;### one for foreground, and the other for background.                ###
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
;###  -- Do not pass numbers greater that 3 as color parameters, as   ###
;###     they are used as indexes in a color table, and results may   ###
;###     be unpredictable                                             ###
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
;### MEASURES                                                         ###
;### MEMORY: 126 bytes (4 table + 122 code)                           ###
;### TIME:                                                            ###
;###  Best case  = 1198 cycles (299,50 us)                            ###
;###  Worst case = 1318 cycles (329,50 us)                            ###
;########################################################################
;

;;
;; Color tables
;;
.bndry 16   ;; Make this vector start at a 16-byte aligned address to be able to use 8-bit arithmetic with pointers
;; Change!!!
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

   ;; Set up foreground and background colours for printing (getting them from tables)
   ;; -- Basically, we need to get values of the 2 bits that should be enabled when the a pixel is present
   LD  A, L                    ;; [ 4] A = ASCII code of the character
   LD  (dcm1_asciiHL+1), A     ;; [13] Save ASCII code of the character as data of a later "LD HL, #data" instruction. This is faster than pushing and popping to the stack

   LD  HL, #dc_mode1_ct        ;; [10] HL points to the start of the color table
   LD  A, L                    ;; [ 4] HL += C (Foreground color is an index in the color table, so we increment HL by C bytes,
   ADD C                       ;; [ 4]          which makes HL point to the Foreground color bits we need. This is valid because
   LD  L, A                    ;; [ 4]          table is 4-bytes aligned and we just need to increment L, as H won't change)
   SUB C                       ;; [ 4] A = L again (Make A save the original value of L, to use it again later with Background color)
   LD  C, (HL)                 ;; [ 7] C = Foreground color bits
   ADD B                       ;; [ 4] HL += B (We increment HL with Background color index, same as we did earlier with Foreground color C)
   LD  L, A                    ;; [ 4]
   LD  A, (HL)                 ;; [ 7] A = Background color bits
   LD (dcm1_drawForeground-2), A ;; [13] Modify Inmediate value of "OR #0" to set it with the foreground color bits
   LD  A, C                    ;; [ 4] A = Foreground color bits (Previously stored into C)
   LD (dcm1_drawForeground+1), A ;; [13] Modify Inmediate value of "OR #0" to set it with the background color bits

   ;; Make HL point to the starting byte of the desired character,
   ;; That is ==> HL = 8*(ASCII code) + char0_ROM_address 
dcm1_asciiHL:
   LD   HL, #0                 ;; [10] HL = ASCII code (H=0, L=ASCII code). 0 is a placeholder to be filled up with the ASCII code value

   ADD  HL, HL                 ;; [11] HL = HL * 8  (8 bytes each character)
   ADD  HL, HL                 ;; [11]
   ADD  HL, HL                 ;; [11]
   LD   BC, #char0_ROM_address ;; [10] BC = 0x3800, Start ROM memory address of Char 0
   ADD  HL, BC                 ;; [11] HL += BC (Now HL Points to the start of the Character definition in ROM memory)

   ;; Enable Lower ROM during char copy operation, with interrupts disabled 
   ;; to prevent firmware messing things up
   LD   A,(gfw_mode_rom_status);; [13] A = mode_rom_status (present value)
   AND  #0b11111011            ;; [ 7] bit 3 of A = 0 --> Lower ROM enabled (0 means enabled)
   LD   B, #GA_port_byte       ;; [ 7] B = Gate Array Port (0x7F)
   DI                          ;; [ 4] Disable interrupts to prevent firmware from taking control while Lower ROM is enabled
   OUT (C), A                  ;; [12] GA Command: Set Video Mode and ROM status (100)

   ;; Do this each pixel line (8-pixels)
dcm1_nextline:
   LD C, (HL)                  ;; [ 7] C = Next Character pixel line definition (8 bits defining 0=backgound color, 1=foreground)
   PUSH HL                     ;; [11] Save HL register to be able to use it as temporal storage
   LD L, #2                    ;; [ 7] L=2 bytes per line

   ;; Do this each video-memory-byte (4-pixels)
dcm1_next4pixels:
   XOR A                       ;; [ 4] A = 0 (A will hold the values of the next 4 pixels in video memory. They will be calculated as Character is read)
   LD  B, #4                   ;; [ 7] B = 4 (4 pixels for each byte)

   ;; Do this each pixel inside a byte (each byte has 4 pixels)
dcm1_nextpixel:
   SLA C                       ;; [ 8] Shift C (Char pixel line) left to know about Bit7 (next pixel) that will turn on/off the carry flag
   JP C, dcm1_drawForeground   ;; [10] IF Carry, bit 7 was a 1, what means foreground color
   OR  #00                     ;; [ 7] Bit7=0, draw next pixel of Background color
   .db #0xDA  ; JP C, xxxx     ;; [10] As carry is never set after an OR, this jump will never be done, and next instruction will be 3 bytes from here (effectively jumping over OR xx without a jump) 
dcm1_drawForeground:
   OR  #00                     ;; [ 7] Bit7=1, draw next pixel of Foreground color
   RLCA                        ;; [ 4] Rotate A 1 bit left to prepare it for inserting next pixel color (same 2 bits will be operated but, as long as A is rotated first, we effectively operate on next 2 bits to the right)
   DJNZ dcm1_nextpixel         ;; [13/8] IF B!=0, continue calculations with next pixel

   LD (DE), A                  ;; [ 7] Save the 4 recently calculated pixels into video memory

   DEC L                       ;; [ 4] L-- (1 byte less to finish this line)
   JP Z, dcm1_endpixelline     ;; [10] If L=0, we have finished the line

   INC DE                      ;; [ 6] ... and point to next 4 pixels in video memory (next byte, DE++)

   JP dcm1_next4pixels         ;; [10] Continue with next 4 pixels

dcm1_endpixelline:
   ;; Move to next pixel-line definition of the character
   POP HL                      ;; [10] Restore HL previous value from stack
   INC L                       ;; [ 4] Next pixel Line (characters are 8-byte-aligned in memory, so we only need to increment L, as H will not change)
   LD  A, L                    ;; [ 4] IF next pixel line corresponds to a new character (this is, we have finished drawing our character),
   AND #0x07                   ;; [ 7] ... then L % 8 == 0, as it is 8-byte-aligned. 
   JP Z, dcm1_end_printing     ;; [10] IF L%8 = 0, we have finished drawing the character, else, proceed to next line

   ;; Prepare to copy next line 
   ;;  -- Move DE pointer to the next pixel line on the video memory
   DEC DE                      ;; [ 6] DE-- : Restore DE to previous position in memory (it has moved 1 byte forward)
   LD  A, D                    ;; [ 4] Start of next pixel line normally is 0x0800 bytes away.
   ADD #0x08                   ;; [ 7]    so we add it to DE (just by adding 0x08 to D)
   LD  D, A                    ;; [ 4]
   AND #0x38                   ;; [ 7] We check if we have crossed memory boundary (every 8 pixel lines)
   JP NZ, dcm1_nextline        ;; [10]  by checking the 4 bits that identify present memory line. If 0, we have crossed boundaries
dcm1_8bit_boundary_crossed:
   LD  A, E                    ;; [ 4] DE = DE + C050h 
   ADD #0x50                   ;; [ 7]   -- Relocate DE pointer to the start of the next pixel line 
   LD  E, A                    ;; [ 4]   -- in video memory
   LD  A, D                    ;; [ 4]
   ADC #0xC0                   ;; [ 7]
   LD  D, A                    ;; [ 4]
   JP  dcm1_nextline           ;; [10] Jump to continue with next pixel line

dcm1_end_printing:
   ;; After finishing character printing, restore previous ROM and Interrupts status
   LD   A,(gfw_mode_rom_status);; [13] A = mode_rom_status (present saved value)
   LD   B,  #GA_port_byte      ;; [ 7] B = Gate Array Port (0x7F)
   OUT (C), A                  ;; [12] GA Command: Set Video Mode and ROM status (100)
   EI                          ;; [ 4] Enable interrupts

   RET                         ;; [10] Return