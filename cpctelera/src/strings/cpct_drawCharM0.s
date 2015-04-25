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

;
;########################################################################
;## FUNCTION: _cpct_drawROMCharM0                                     ###
;########################################################################
;### This function reads a character from ROM and draws it at a given ###
;### point on the video memory (byte-aligned), assumming screen is    ###
;### configured for MODE 0. It prints the character in 2 colors(PENs) ###
;### one for foreground, and the other for background.                ###
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
;### MEASURES (Way 2 for parameter retrieval from stack)              ###
;### MEMORY: 136 bytes (16 table + 120 code)                          ###
;### TIME:                                                            ###
;###  Best case  = 3744 cycles ( 936.00 us)                           ###
;###  Worst case = 4424 cycles (1106.00 us)                           ###
;########################################################################
;


;;
;; Color table
;;
;; Bndry does not work when file is linked after being compiled.
;;.bndry 16   ;; Make this vector start at a 16-byte aligned address to be able to use 8-bit arithmetic with pointers
dc_mode0_ct: .db 0x00, 0x40, 0x04, 0x44, 0x10, 0x50, 0x14, 0x54, 0x01, 0x41, 0x05, 0x45, 0x11, 0x51, 0x15, 0x55

_cpct_drawCharM0::
   ;; GET Parameters from the stack 
.if let_disable_interrupts_for_function_parameters
   ;; Way 1: Pop + Restoring SP. Faster, but consumes 4 bytes more, and requires disabling interrupts
   ld (dcm0_restoreSP+1), sp   ;; [20] Save SP into placeholder of the instruction LD SP, 0, to quickly restore it later.
   di                          ;; [ 4] Disable interrupts to ensure no one overwrites return address in the stack
   pop  af                     ;; [10] AF = Return Address
   pop  de                     ;; [10] DE = Destination address (Video memory location where character will be printed)
   pop  bc                     ;; [10] BC = Colors (B=Background color, C=Foreground color) 
   pop  hl                     ;; [10] HL = ASCII code of the character (H=??, L=ASCII code)
dcm0_restoreSP:
   ld sp, #0                   ;; [10] -- Restore Stack Pointer -- (0 is a placeholder which is filled up with actual SP value previously)
   ei                          ;; [ 4] Enable interrupts again
.else 
   ;; Way 2: Pop + Push. Just 6 cycles more, but does not require disabling interrupts
   pop  af                     ;; [10] AF = Return Address
   pop  de                     ;; [10] DE = Destination address (Video memory location where character will be printed)
   pop  bc                     ;; [10] BC = Colors (B=Background color, C=Foreground color) 
   pop  hl                     ;; [10] HL = ASCII code of the character (H=??, L=ASCII code)
   push hl                     ;; [11] Restore Stack status pushing values again
   push bc                     ;; [11] (Interrupt safe way, 6 cycles more)
   push de                     ;; [11]
   push af                     ;; [11]
.endif

   ;; Set up foreground and background colours for printing (getting them from tables)
   ;; -- Basically, we need to get values of the 4 bits that should be enabled when the a pixel is present
   LD  A, L                    ;; [ 4] A = ASCII code of the character

_cpct_drawCharM0_asm::
   LD  (dcm0_asciiHL+1), A     ;; [13] Save ASCII code of the character as data of a later "LD HL, #data" instruction. This is faster than pushing and popping to the stack because H needs to be resetted

   LD  HL, #dc_mode0_ct        ;; [10] HL points to the start of the color table

   ; This code only works if color table is 16-byte aligned and saves up to 34 cycles
   ;LD  A, L                   ;; [ 4] HL += C (Foreground color is an index in the color table, so we increment HL by C bytes,
   ;ADD C                      ;; [ 4]          which makes HL point to the Foreground color bits we need. This is valid because
   ;LD  L, A                   ;; [ 4]          table is 16-bytes aligned and we just need to increment L, as H won't change)
   ;SUB C                      ;; [ 4] A = L again (Make A save the original value of L, to use it again later with Background color)
   ;LD  C, (HL)                ;; [ 7] C = Foreground color bits
   ;ADD B                      ;; [ 4] HL += B (We increment HL with Background color index, same as we did earlier with Foreground color C)
   ;LD  L, A                   ;; [ 4]

   ld  a, l                    ;; [ 4] HL += C (Let HL point to the concrete color in the table:
   add c                       ;; [ 4]          HL points initial to the start of the table and C is the Foreground PEN number,
   ld  l, a                    ;; [ 4]          so HL+C is the memory location of the color bits we need).
   adc h                       ;; [ 4]
   sub l                       ;; [ 4]
   ld  h, a                    ;; [ 4]

   ld  c, (hl)                 ;; [ 7] C = Foreground color bits
   ld  hl, #dc_mode0_ct        ;; [10] HL points again to the start of the color table
   ld  a, l                    ;; [ 4] HL += B (Let HL point to the concrete color in the table:
   add b                       ;; [ 4]          HL points initial to the start of the table and B is the Background PEN number,
   ld  l, a                    ;; [ 4]          so HL+B is the memory location of the color bits we need).
   adc h                       ;; [ 4]
   sub l                       ;; [ 4]
   ld  h, a                    ;; [ 4]

   LD  A, (HL)                 ;; [ 7] A = Background color bits
   LD (dcm0_drawForeground_0-2), A ;; [13] <| Modify Inmediate value of "OR #0" to set it with the background color bits
   LD (dcm0_drawForeground_1-2), A ;; [13] <|  (We do it 2 times, as 2 bits = 2 pixels = 1 byte in video memory)
   LD  A, C                        ;; [ 4] A = Foreground color bits (Previously stored into C)
   LD (dcm0_drawForeground_0+1), A ;; [13] <| Modify Inmediate value of "OR #0" to set it with the foreground color bits
   LD (dcm0_drawForeground_1+1), A ;; [13] <|  (We do it 2 times too)

   ;; Make HL point to the starting byte of the desired character,
   ;; That is ==> HL = 8*(ASCII code) + char0_ROM_address 
dcm0_asciiHL:
   LD   HL, #0                 ;; [10] HL = ASCII code (H=0, L=ASCII code). 0 is a placeholder to be filled up with the ASCII code value

   ADD  HL, HL                 ;; [11] HL = HL * 8  (8 bytes each character)
   ADD  HL, HL                 ;; [11]
   ADD  HL, HL                 ;; [11]
   LD   BC, #char0_ROM_address ;; [10] BC = 0x3800, Start ROM memory address of Char 0
   ADD  HL, BC                 ;; [11] HL += BC (Now HL Points to the start of the Character definition in ROM memory)

   ;; Enable Lower ROM during char copy operation, with interrupts disabled 
   ;; to prevent firmware messing things up
   LD   A,(cpct_mode_rom_status);; [13] A = mode_rom_status (present value)
   AND  #0b11111011            ;; [ 7] bit 3 of A = 0 --> Lower ROM enabled (0 means enabled)
   LD   B, #GA_port_byte       ;; [ 7] B = Gate Array Port (0x7F)
   DI                          ;; [ 4] Disable interrupts to prevent firmware from taking control while Lower ROM is enabled
   OUT (C), A                  ;; [12] GA Command: Set Video Mode and ROM status (100)

   ;; Do this for each pixel line (8-pixels)
dcm0_nextline:
   LD C, (HL)                  ;; [ 7] C = Next Character pixel line definition (8 bits defining 0=background color, 1=foreground)
   LD B, #4                    ;; [ 7] L=4 bytes per line
   PUSH DE                     ;; [11] Save place where DE points now (start of the line) to be able to use it later to jump to next line

   ;; Do this for each video-memory-byte (2-pixels)
dcm0_next2pixels:
   XOR A                       ;; [ 4] A = 0 (A will hold the values of the next 2 pixels in video memory. They will be calculated as Character is read)

   ;; Transform first pixel
   SLA C                       ;; [ 8] Shift C (Char pixel line) left to know about Bit7 (next pixel) that will turn on/off the carry flag
   JP C, dcm0_drawForeground_0 ;; [10] IF Carry, bit 7 was a 1, what means foreground color
   OR  #00                     ;; [ 7] Bit7=0, draw next pixel of Background color
   .db #0xDA  ; JP C, xxxx     ;; [10] As carry is never set after an OR, this jump will never be done, and next instruction will be 3 bytes from here (effectively jumping over OR xx without a jump) 
dcm0_drawForeground_0:
   OR  #00                     ;; [ 7] Bit7=1, draw next pixel of Foreground color
   RLCA                        ;; [ 4] Rotate A 1 bit left to prepare it for inserting next pixel color (same 2 bits will be operated but, as long as A is rotated first, we effectively operate on next 2 bits to the right)

   ;; Transform second pixel
   SLA C                       ;; [ 8] <\ Do the same as first pixel, for second one
   JP C, dcm0_drawForeground_1 ;; [10]  |
   OR  #00                     ;; [ 7]  |
   .db #0xDA  ; JP C, xxxx     ;; [10]  |
dcm0_drawForeground_1:         ;;       |
   OR  #00                     ;; [ 7] </

   LD (DE), A                  ;; [ 7] Save the 2 recently calculated pixels into video memory
   INC DE                      ;; [ 6] ... and point to next 2 pixels in video memory (next byte, DE++)

   DJNZ dcm0_next2pixels       ;; [13/8] If B!=0, continue with next 2 pixels of this pixel-line

dcm0_endpixelline:
   POP DE                      ;; [10] Recover previous DE position, to use it now for jumping to next video memory line

   ;; Move to next pixel-line definition of the character
   INC L                       ;; [ 4] Next pixel Line (characters are 8-byte-aligned in memory, so we only need to increment L, as H will not change)
   LD  A, L                    ;; [ 4] IF next pixel line corresponds to a new character (this is, we have finished drawing our character),
   AND #0x07                   ;; [ 7] ... then L % 8 == 0, as it is 8-byte-aligned. 
   JP Z, dcm0_end_printing     ;; [10] IF L%8 = 0, we have finished drawing the character, else, proceed to next line

   ;; Prepare to copy next line 
   ;;  -- Move DE pointer to the next pixel line on the video memory
   LD  A, D                    ;; [ 4] Start of next pixel line normally is 0x0800 bytes away.
   ADD #0x08                   ;; [ 7]    so we add it to DE (just by adding 0x08 to D)
   LD  D, A                    ;; [ 4]
   AND #0x38                   ;; [ 7] We check if we have crossed memory boundary (every 8 pixel lines)
   JP NZ, dcm0_nextline        ;; [10]  by checking the 4 bits that identify present memory line. If 0, we have crossed boundaries
dcm0_8bit_boundary_crossed:
   LD  A, E                    ;; [ 4] DE = DE + C050h 
   ADD #0x50                   ;; [ 7]   -- Relocate DE pointer to the start of the next pixel line 
   LD  E, A                    ;; [ 4]   -- in video memory
   LD  A, D                    ;; [ 4]
   ADC #0xC0                   ;; [ 7]
   LD  D, A                    ;; [ 4]
   JP  dcm0_nextline           ;; [10] Jump to continue with next pixel line

dcm0_end_printing:
   ;; After finishing character printing, restore previous ROM and Interrupts status
   LD   A,(cpct_mode_rom_status);; [13] A = mode_rom_status (present saved value)
   LD   B,  #GA_port_byte      ;; [ 7] B = Gate Array Port (0x7F)
   OUT (C), A                  ;; [12] GA Command: Set Video Mode and ROM status (100)
   EI                          ;; [ 4] Enable interrupts

   RET                         ;; [10] Return
