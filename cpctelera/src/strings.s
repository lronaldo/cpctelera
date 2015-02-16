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
;###  Best case  = 4358 cycles (1089,50 us)                           ###
;###  Worst case = 5038 cycles (1259,50 us)                           ###
;########################################################################
;

;;
;; Color table
;;
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
   POP  HL                     ;; [10] HL = ASCII code of the character (H=??, L=ASCII code)
dcm1_restoreSP:
   LD SP, #0                   ;; [10] -- Restore Stack Pointer -- (0 is a placeholder which is filled up with actual SP value previously)
   EI                          ;; [ 4] Enable interrupts again

   ;; Set up foreground and background colours for printing (getting them from tables)
   ;; -- Basically, we need to get values of the 2 bits that should be enabled when the a pixel is present
   LD  A, L                    ;; [ 4] A = ASCII code of the character
   LD  (dcm1_asciiHL+1), A     ;; [13] Save ASCII code of the character as data of a later "LD HL, #data" instruction. This is faster than pushing and popping to the stack because H needs to be resetted

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


;
;########################################################################
;## FUNCTION: _cpct_drawROMCharM1_fast                                ###
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
;### MEMORY: 351 bytes                                                ###
;### TIME:                                                            ###
;###  Best case  = 1960 cycles (490,00 us)                            ###
;###  Worst case = 2678 cycles (669,50 us)                            ###
;########################################################################
;

.globl _cpct_drawROMCharM1_fast
_cpct_drawROMCharM1_fast::
   ;; GET Parameters from the stack (Pop + Restoring SP)
   LD (dcm1f_restoreSP+1), SP  ;; [20] Save SP into placeholder of the instruction LD SP, 0, to quickly restore it later.
   DI                          ;; [ 4] Disable interrupts to ensure no one overwrites return address in the stack
   POP  AF                     ;; [10] AF = Return Address
   POP  DE                     ;; [10] DE = Destination address (Video memory location where character will be printed)
   POP  BC                     ;; [10] BC = Colors (B=Background color, C=Foreground color) 
   POP  HL                     ;; [10] HL = ASCII code of the character (H=0, L=ASCII code)
dcm1f_restoreSP:
   LD SP, #0                   ;; [10] -- Restore Stack Pointer -- (0 is a placeholder which is filled up with actual SP value previously)
   EI                          ;; [ 4] Enable interrupts again

   ;; Save ASCII code of the character to get it later
   LD  A, L                    ;; [ 4] A = ASCII code of the character
   LD  (dcm1f_asciiHL+1), A    ;; [13] Save ASCII code of the character as data of a later "LD HL, #data" instruction. This is faster than pushing and popping to the stack because H needs to be resetted

   ;;---------------------------------------------------------------------------------------------------
   ;; Set up foreground and background code that does the color mixing into bytes for video memory
   ;;---------------------------------------------------------------------------------------------------

   ;;
   ;; ## PART 1 ## : Insert code for background color generation
   ;;   This code inserts 2 pieces of dynamic code (Into dcm1f_start_b1bg and dcm1f_start_b2bg) that
   ;;   insert the background color of each character line into the bytes that will be written to 
   ;;   video memory. The code inserted depends on the background color: there are 4 possible pieces of
   ;;   code, so we use register B (background color) to select the piece of code to insert.
   ;;
   INC B                       ;; [ 4] Set Background color between 1 and 4
   DJNZ dcm1f_bg01             ;; [13/8] If BGColor=00, do not jump, else, jump to BGColor=01 test

dcm1f_bg00: 
   ;; Background color is 00. Insert Dynamic code into placeholders.
   ;;  *) This is the dynamic code that will be inserted into dcm1f_start_b(1/2)bg (3 bytes):
   ;;  Assembler    ; Mach.Code  Cycles
   ;;   XOR A       ; [ AF   ]   [ 4] A = 0 (As background is 0, no pixel is to be lightened up)
   ;;   JR  $+7     ; [ 1805 ]   [12] Jump to the end of the dynamic code section
   LD  A, #0xAF                ;; [ 7] A = AFh    (AF    = XOR A)
   LD (dcm1f_start_b2bg), A    ;; [13] <<= Insert XOR A into code for managing byte 2
   LD (dcm1f_start_b1bg), A    ;; [13] <<= Insert XOR A into code for managing byte 1
   LD  HL, #0x0518             ;; [10] HL = 0518h (18 05 = JR $+7)
   LD (dcm1f_start_b2bg+1), HL ;; [16] <<= Insert JR $+7 into code for managing byte 2
   LD (dcm1f_start_b1bg+1), HL ;; [16] <<= Insert JR $+7 into code for managing byte 1
   JP dcm1f_fg_dyncode         ;; [10] Finish background dynamic code inserting

dcm1f_bg01:
   ;; Background color is > 00 (so 01 to 11). All this 3 sections of code have at least 4 RRCA's
   ;; for one of the bytes, so we prepare HL for all of them now, avoiding unuseful repetitions
   LD  HL, #0x0F0F             ;; [10] HL = 0F0Fh (0F = RRCA)
   DJNZ dcm1f_bg10             ;; [13/8] If BGColor=01, do not jump, else, jump to BGColor=10 test

   ;; Background color is 01. Insert Dynamic code into placeholders.
   ;;  *) This is the dynamic code that will be inserted into dcm1f_start_b1bg (4 bytes):
   ;;  Assembler    ; Mach.Code  Cycles
   ;;   AND #0xF0   ; [ E6F0 ]    [ 7] Only first 4 bits are lightened, as this bits represent the bit 0 of the 4 first pixels (that has to be 1 for pixels activated)
   ;;   JR  $+6     ; [ 1804 ]    [12] Jump to the end of the dynamic code section
   ;;  *) And this is the dynamic code that will be inserted into dcm1f_start_b2bg (7 bytes):
   ;;  Assembler    ; Mach.Code  Cycles
   ;;   RRCA        ; [ 0F   ]    [ 4] Move low nibble of A to high nibble of A rotating bits
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- The 4 bits of the high nibble represent the bit 0 for the 4 pixels
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- And for color 01 the bit 0 is the one that has to be active
   ;;   RRCA        ; [ 0F   ]    [ 4]
   ;;   AND #0xF0   ; [ E6F0 ]    [ 7] Set low nibble of A to 0 (as it represents bit 1 for our 4 pixels, an it should be always 0 for color 01)
   ;;   JR C, $xx   ; [ 38xx ]    [ 7] Create a conditional jump that will never be done (carry is always false). This is only 7 cycles (2 NOP instructions would be 8, and we do not overwritte the last byte)
   LD (dcm1f_start_b2bg), HL   ;; [16] <<= Insert RRCA;RRCA;RRCA;RRCA into code for managing byte 2
   LD (dcm1f_start_b2bg+2), HL ;; [16]
   LD  HL, #0xF0E6             ;; [10] HL = F0E6h (E6 F0 = AND F0h)
   LD (dcm1f_start_b1bg), HL   ;; [16] <<= Insert AND #0xF0 into code for managing byte 1 and byte 2
   LD (dcm1f_start_b2bg+4), HL ;; [16]
   LD  HL, #0x0418             ;; [10] HL = 0418h (18 04 = JR $+6)
   LD (dcm1f_start_b1bg+2), HL ;; [16] <<= Insert JR $+6 into code for managing byte 1
   LD   A, #0x38               ;; [ 7] A = 38h (38 = JR C, xx)
   LD (dcm1f_start_b2bg+6), A  ;; [13] <<= Insert JR C, xx into code for managing byte 2
   JP dcm1f_fg_dyncode         ;; [10] Finish background dynamic code inserting
dcm1f_bg10:
   ;; Background color is > 01 (so 10 or 11). All this 2 sections of code have at least 4 RRCA's
   ;; in the management of the first byte. Then we insert it now, to avoid unuseful repetitions
   LD (dcm1f_start_b1bg), HL   ;; [16] <<= Insert RRCA;RRCA;RRCA;RRCA into code for managing byte 1
   LD (dcm1f_start_b1bg+2), HL ;; [16]
   DJNZ dcm1f_bg11             ;; [13/8] If BGColor=10, do not jump, else, jump to BGColor=11
   ;; Background color is 01. Insert Dynamic code into placeholders.
   ;;  *) This is the dynamic code that will be inserted into dcm1f_start_b1bg (7 bytes):
   ;;  Assembler    ; Mach.Code  Cycles
   ;;   RRCA        ; [ 0F   ]    [ 4] Move low nibble of A to high nibble of A rotating bits
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- The 4 bits of the high nibble represent the bit 0 for the 4 pixels
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- And for color 01 the bit 0 is the one that has to be active
   ;;   RRCA        ; [ 0F   ]    [ 4]
   ;;   AND #0xF0   ; [ E6F0 ]    [ 7] Set low nibble of A to 0 (as it represents bit 1 for our 4 pixels, an it should be always 0 for color 01)
   ;;   JR C, $xx   ; [ 38xx ]    [ 7] Create a conditional jump that will never be done (carry is always false). This is only 7 cycles (2 NOP instructions would be 8, and we do not overwritte the last byte)
   ;;  *) And this is the dynamic code that will be inserted into dcm1f_start_b2bg (4 bytes):
   ;;  Assembler    ; Mach.Code  Cycles
   ;;   AND #0xF0   ; [ E6F0 ]    [ 7] Only first 4 bits are lightened, as this bits represent the bit 0 of the 4 first pixels (that has to be 1 for pixels activated)
   ;;   JR  $+6     ; [ 1804 ]    [12] Jump to the end of the dynamic code section
   LD  HL, #0x0FE6             ;; [10] HL = 0FE6h (E6 0F = AND 0Fh)
   LD (dcm1f_start_b2bg), HL   ;; [16]
   LD (dcm1f_start_b1bg+4), HL ;; [16]
   LD  HL, #0x0418             ;; [10] HL = 0418h (18 04 = JR $+6)
   LD (dcm1f_start_b2bg+2), HL ;; [16]
   LD   A, #0x38               ;; [ 7] A = 38h (38 = JR C, xx)
   LD (dcm1f_start_b1bg+6), A  ;; [13] 
   JP dcm1f_fg_dyncode         ;; [10]
dcm1f_bg11:
   ;; Background color is 11. Insert Dynamic code into placeholders.
   ;;  *) This is the dynamic code that will be inserted into dcm1f_start_b(1/2)bg (8 bytes):
   ;;  Assembler    ; Mach.Code  Cycles
   ;;   RRCA        ; [ 0F   ]    [ 4] Interchange low and high nibble bits of A
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- We want to duplicate low/high nibble of A (depending on which byte are we processing)
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- We have an invertd copy of A in C. By interchanging nibbles of A we can
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- mix A and C to get in A the same two nibbles, which is same value for bits 0 and 1 of each pixel (color 11)
   ;;   XOR C       ; [ A9   ]    [ 4] Mix A and C to get the same values at the two nibbles of A
   ;;     OR #0xF0  ; [ F6F0 ] b1 [ 7]  -- This is only for byte 1 (dcm1f_start_b1bg)
   ;;     OR #0x0F  ; [ F60F ] b2 [ 7]  -- This is only for byte 2 (dcm1f_start_b2bg)
   ;;   XOR C       ; [ A9   ]    [ 4] ;; This last piece of code is static, as it never changes, so no need to insert it
   LD (dcm1f_start_b2bg), HL   ;; [16] <<= Insert RRCA;RRCA;RRCA;RRCA into code for managing byte 2
   LD (dcm1f_start_b2bg+2), HL ;; [16]
   LD  HL, #0xF6A9             ;; [10] HL = E6A9h (A9 = XOR C, F6 = OR xx (value in next byte))
   LD (dcm1f_start_b2bg+4), HL ;; [16] <<= Insert XOR C; OR xx into code for managing bytes 1 and 2
   LD (dcm1f_start_b1bg+4), HL ;; [16]
   LD   A, #0x0F               ;; [ 7] A = 0Fh (0F = Value for previous OR in first byte)
   LD (dcm1f_start_b2bg+6), A  ;; [13] <<= Insert 0F as inmediate value for previous OR
   CPL                         ;; [ 4] A = F0h (F0 = Value for previous OR in second byte)
   LD (dcm1f_start_b1bg+6), A  ;; [13] <<= Insert F0 as inmediate value for previous OR

dcm1f_fg_dyncode:
   LD  B, C                    ;; [ 4] B = Foreground color
   INC B                       ;; [ 4] Set Foreground color between 1 and 4
   DJNZ dcm1f_fg01             ;; [13/8] If FGColor=00, do not jump, else, jump to BGColor=01 test
dcm1f_fg00:
   ;; Background color is 00. Insert Dynamic code into placeholders.
   LD  A, #0xAF                ;; [ 7] A = AFh    (AF    = XOR A)
   LD (dcm1f_start_b2fg), A    ;; [13]
   LD (dcm1f_start_b1fg), A    ;; [13]
   LD  HL, #0x0518             ;; [10] HL = 0518h (18 05 = JR $+7)
   LD (dcm1f_start_b2fg+1), HL ;; [16]
   LD (dcm1f_start_b1fg+1), HL ;; [16]
   JP dcm1f_asciiHL            ;; [10]
dcm1f_fg01:
   LD  HL, #0x0F0F             ;; [10] HL = 0F0Fh (0F = RRCA)
   DJNZ dcm1f_fg10             ;; [13/8] If BGColor=01, do not jump, else, jump to BGColor=10 test
   ;; Background color is 01. Insert Dynamic code into placeholders.
   LD (dcm1f_start_b2fg), HL   ;; [16]
   LD (dcm1f_start_b2fg+2), HL ;; [16]
   LD  HL, #0xF0E6             ;; [10] HL = F0E6h (E6 F0 = AND F0h)
   LD (dcm1f_start_b1fg), HL   ;; [16]
   LD (dcm1f_start_b2fg+4), HL ;; [16]
   LD  HL, #0x0418             ;; [10] HL = 0418h (18 04 = JR $+6)
   LD (dcm1f_start_b1fg+2), HL ;; [16]
   LD   A, #0x38               ;; [ 7] HL = 38h (38 = JR C, xx)
   LD (dcm1f_start_b2fg+6), A  ;; [13]
   JP dcm1f_asciiHL            ;; [10]
dcm1f_fg10:
   LD (dcm1f_start_b1fg), HL   ;; [16] <<- RRCA * 4
   LD (dcm1f_start_b1fg+2), HL ;; [16]
   DJNZ dcm1f_fg11             ;; [13/8] If BGColor=10, do not jump, else, jump to BGColor=11
   ;; Background color is 10. Insert Dynamic code into placeholders.
   LD (dcm1f_start_b1fg), HL   ;; [16]
   LD (dcm1f_start_b1fg+2), HL ;; [16]
   LD  HL, #0x0FE6             ;; [10] HL = 0FE6h (E6 0F = AND 0Fh)
   LD (dcm1f_start_b2fg), HL   ;; [16]
   LD (dcm1f_start_b1fg+4), HL ;; [16]
   LD  HL, #0x0418             ;; [10] HL = 0418h (18 04 = JR $+6)
   LD (dcm1f_start_b2fg+2), HL ;; [16]
   LD   A, #0x38               ;; [ 7] A = 38h (38 = JR C, xx)
   LD (dcm1f_start_b1fg+6), A  ;; [13]
   JP dcm1f_asciiHL            ;; [10]
dcm1f_fg11:
   ;; Background color is 11. Insert Dynamic code into placeholders.
   LD (dcm1f_start_b2fg), HL   ;; [16] <<- RRCA * 4
   LD (dcm1f_start_b2fg+2), HL ;; [16]
   LD  HL, #0xE6A9             ;; [10] HL = E6A9h (A9 = XOR C, E6 = AND xx (value in next byte))
   LD (dcm1f_start_b2fg+4), HL ;; [16]
   LD (dcm1f_start_b1fg+4), HL ;; [16]
   LD   A, #0xF0               ;; [ 7] A = F0h (F0 = Value for previous AND)
   LD (dcm1f_start_b2fg+6), A  ;; [13]
   CPL                         ;; [ 4] A = 0Fh (0F = Value for previous AND in second byte)
   LD (dcm1f_start_b1fg+6), A  ;; [13]

   ;; Make HL point to the starting byte of the desired character,
   ;; That is ==> HL = 8*(ASCII code) + char0_ROM_address 
dcm1f_asciiHL:
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

   ;;; Get next pixel-line definition
dcm1f_nextPixelLine: 
   LD A, (HL)                  ;; [ 7] C = Next Character pixel line definition (8 bits defining 

dcm1f_calculate_two_bytes:
   LD  C, A    ;  4

;; First Byte FG Color (8 bytes of dynamic code)
dcm1f_start_b1fg:
   .ds 7
   XOR C       ; 4

dcm1f_end_b1fg:
   LD  B, A    ; 4
   LD  A, C    ; 4
   CPL         ; 4  A = ^A

;; First Byte BG Color (8 bytes of dynamic code)
dcm1f_start_b1bg:
   .ds 7
   XOR C       ; 4

dcm1f_end_b1bg:
   ;; Save First Byte
   OR  B           ; 4
   LD  (DE), A     ; 7
   INC DE          ; 6

   ;; Set up values for working with 2nd byte
   LD  A, C        ; 4

;; Second Byte FG Color (8 bytes of dynamic code)
dcm1f_start_b2fg:
   .ds 7
   XOR C       ; 4

dcm1f_end_b2fg:
   LD  B, A    ; 4
   LD  A, C    ; 4
   CPL         ; 4  A = ^A

;; Second Byte BG Color (8 bytes of dynamic code)
dcm1f_start_b2bg:
   .ds 7
   XOR C       ; 4

dcm1f_end_b2bg:
   ;; Save Second Byte
   OR  B           ; 4
   LD  (DE), A     ; 7

;; End of this pixel line, jump to next one or end
   INC L                       ;; [ 4] Next pixel Line (characters are 8-byte-aligned in memory, so we only need to increment L, as H will not change)
   LD  A, L                    ;; [ 4] IF next pixel line corresponds to a new character (this is, we have finished drawing our character),
   AND #0x07                   ;; [ 7] ... then L % 8 == 0, as it is 8-byte-aligned. 
   JP Z, dcm1f_end_printing    ;; [10] IF L%8 = 0, we have finished drawing the character, else, proceed to next line

   ;; Prepare to copy next line 
   ;;  -- Move DE pointer to the next pixel line on the video memory
   DEC DE                      ;; [ 6] DE-- : Restore DE to previous position in memory (it has moved 1 byte forward)
   LD  A, D                    ;; [ 4] Start of next pixel line normally is 0x0800 bytes away.
   ADD #0x08                   ;; [ 7]    so we add it to DE (just by adding 0x08 to D)
   LD  D, A                    ;; [ 4]
   AND #0x38                   ;; [ 7] We check if we have crossed memory boundary (every 8 pixel lines)
   JP NZ, dcm1f_nextPixelLine  ;; [10]  by checking the 4 bits that identify present memory line. If 0, we have crossed boundaries
dcm1f_8bit_boundary_crossed:
   LD  A, E                    ;; [ 4] DE = DE + C050h 
   ADD #0x50                   ;; [ 7]   -- Relocate DE pointer to the start of the next pixel line 
   LD  E, A                    ;; [ 4]   -- in video memory
   LD  A, D                    ;; [ 4]
   ADC #0xC0                   ;; [ 7]
   LD  D, A                    ;; [ 4]
   JP  dcm1f_nextPixelLine     ;; [10] Jump to continue with next pixel line

dcm1f_end_printing:
   ;; After finishing character printing, restore previous ROM and Interrupts status
   LD   A,(gfw_mode_rom_status);; [13] A = mode_rom_status (present saved value)
   LD   B,  #GA_port_byte      ;; [ 7] B = Gate Array Port (0x7F)
   OUT (C), A                  ;; [12] GA Command: Set Video Mode and ROM status (100)
   EI                          ;; [ 4] Enable interrupts

   RET                         ;; [10] Return

;;
;; This is for next routine to draw a character in mode 0
;;
.bndry 16   ;; Make this vector start at a 16-byte aligned address to be able to use 8-bit arithmetic with pointers
;; Change!!!
dc_mode0_ct: .db 0x00, 0x80, 0x08, 0x88, 0x20, 0xA0, 0x28, 0xA8, 0x02, 0x82, 0x0A, 0x8A, 0x22, 0xA2, 0x2A, 0xAA

;; FIRST BYTE
;;
;; 00
;   XOR A       ; 4    [ AF   ]  ==> 3b
;   JR  $+7     ; 12   [ 1805 ]
;;; 01
;   AND #0xF0   ; 7    [ E6F0 ]  ==> 4b
;   JR  $+6     ; 12   [ 1804 ]
;;; 10 
;   RRCA        ; 4    [ 0F   ] ==> 6b
;   RRCA        ; 4    [ 0F   ]
;   RRCA        ; 4    [ 0F   ]
;   RRCA        ; 4    [ 0F   ]
;   AND #0x0F   ; 7    [ E6F0 ]
;   JR C, $xx    ; 7   [ 38xx ] ; Never jump is better than 2 nops
;;; 11
;   RRCA        ; 4    [ 0F   ] ==> 8b
;   RRCA        ; 4    [ 0F   ]
;   RRCA        ; 4    [ 0F   ]
;   RRCA        ; 4    [ 0F   ]
;   XOR C       ; 4    [ A9   ]
; BG-- OR #0xF0   ; 7     [ F6F0 ]
; FG-- AND #0x0F   ; 7    [ E60F ]
;   XOR C       ; 4    [ A9   ]
;
;
;;; SECOND BYTE
;;;
;;; 00
;;; 01
;   RRCA        ; 4    [ 0F   ] ==> 6b
;   RRCA        ; 4    [ 0F   ]
;   RRCA        ; 4    [ 0F   ]
;   RRCA        ; 4    [ 0F   ]
;   AND #0xF0   ; 7    [ E6F0 ]
;   JR C, $xx    ; 7   [ 38xx ] ; Never jump is better than 2 nops
;;; 10 
;   AND #0x0F   ; 7    [ E60F ] ==> 4b
;   JR  $+6     ; 12   [ 1804 ]
;;; 11
;   RRCA        ; 4    [ 0F   ] ==> 8b
;   RRCA        ; 4    [ 0F   ]
;   RRCA        ; 4    [ 0F   ]
;   RRCA        ; 4    [ 0F   ]
;   XOR C       ; 4    [ A9   ]
; BG-- OR #0x0F   ; 7     [ F60F ]
; FG-- AND #0xF0   ; 7    [ E6F0 ]
;   XOR C       ; 4    [ A9   ]
