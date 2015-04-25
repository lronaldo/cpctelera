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
;### MEMORY: 135 bytes (4 table + 131 code)                           ###
;### TIME:                                                            ###
;###  Best case  = 4386 cycles (1096,50 us)                           ###
;###  Worst case = 5066 cycles (1266,50 us)                           ###
;########################################################################
;

;;
;; Color table
;;
;; Bndry does not work when file is linked after being compiled.
;.bndry 4    ;; Make this vector start at a 4-byte aligned address to be able to use 8-bit arithmetic with pointers
dc_mode1_ct: .db 0x00, 0x08, 0x80, 0x88

_cpct_drawCharM1::
   ;; GET Parameters from the stack 
.if let_disable_interrupts_for_function_parameters
   ;; Way 1: Pop + Restoring SP. Faster, but consumes 4 bytes more, and requires disabling interrupts
   ld (dcm1_restoreSP+1), sp   ;; [20] Save SP into placeholder of the instruction LD SP, 0, to quickly restore it later.
   di                          ;; [ 4] Disable interrupts to ensure no one overwrites return address in the stack
   pop  af                     ;; [10] AF = Return Address
   pop  de                     ;; [10] DE = Destination address (Video memory location where character will be printed)
   pop  bc                     ;; [10] BC = Colors (B=Background color, C=Foreground color) 
   pop  hl                     ;; [10] HL = ASCII code of the character (H=??, L=ASCII code)
dcm1_restoreSP:
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
   ;; -- Basically, we need to get values of the 2 bits that should be enabled when the a pixel is present
   ld  a, l                    ;; [ 4] A = ASCII code of the character

_cpct_drawCharM1_asm::
   ld  (dcm1_asciiHL+2), a     ;; [13] Save ASCII code of the character as data of a later "LD HL, #data" instruction. This is faster than pushing and popping to the stack because H needs to be resetted

   ld  hl, #dc_mode1_ct        ;; [10] HL points to the start of the color table
   ;LD  A, L                   ;; [ 4] HL += C (Foreground color is an index in the color table, so we increment HL by C bytes,
   ;ADD C                      ;; [ 4]          which makes HL point to the Foreground color bits we need. This is valid because
   ;LD  L, A                   ;; [ 4]          table is 4-bytes aligned and we just need to increment L, as H won't change)
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
   ld  hl, #dc_mode1_ct        ;; [10] HL points again to the start of the color table
   ld  a, l                    ;; [ 4] HL += B (Let HL point to the concrete color in the table:
   add b                       ;; [ 4]          HL points initial to the start of the table and B is the Background PEN number,
   ld  l, a                    ;; [ 4]          so HL+B is the memory location of the color bits we need).
   adc h                       ;; [ 4]
   sub l                       ;; [ 4]
   ld  h, a                    ;; [ 4]

   ld  a, (hl)                 ;; [ 7] A = Background color bits
   ld (dcm1_drawForeground-2), a ;; [13] Modify Inmediate value of "OR #0" to set it with the background color bits
   ld  a, c                    ;; [ 4] A = Foreground color bits (Previously stored into C)
   ld (dcm1_drawForeground+1), a ;; [13] Modify Inmediate value of "OR #0" to set it with the foreground color bits

   ;; Make HL point to the starting byte of the desired character,
   ;; That is ==> HL = 8*(ASCII code) + char0_ROM_address (0x3800)
dcm1_asciiHL:
   xor  a                      ;; [ 4] A = 0
   or   #0                     ;; [ 7] A = ASCII Value and Resetting carry flag (#0 is a placeholder that will be filled up with ASII value)
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
   ld   a,(cpct_mode_rom_status);; [13] A = mode_rom_status (present value)
   and  #0b11111011            ;; [ 7] bit 3 of A = 0 --> Lower ROM enabled (0 means enabled)
   ld   b, #GA_port_byte       ;; [ 7] B = Gate Array Port (0x7F)
   di                          ;; [ 4] Disable interrupts to prevent firmware from taking control while Lower ROM is enabled
   out (c), a                  ;; [12] GA Command: Set Video Mode and ROM status (100)

   ;; Do this each pixel line (8-pixels)
dcm1_nextline:
   ld c, (hl)                  ;; [ 7] C = Next Character pixel line definition (8 bits defining 0=background color, 1=foreground)
   push hl                     ;; [11] Save HL register to be able to use it as temporal storage
   ld l, #2                    ;; [ 7] L=2 bytes per line

   ;; Do this each video-memory-byte (4-pixels)
dcm1_next4pixels:
   xor a                       ;; [ 4] A = 0 (A will hold the values of the next 4 pixels in video memory. They will be calculated as Character is read)
   ld  b, #4                   ;; [ 7] B = 4 (4 pixels for each byte)

   ;; Do this each pixel inside a byte (each byte has 4 pixels) 
dcm1_nextpixel:
   sla c                       ;; [ 8] Shift C (Char pixel line) left to know about Bit7 (next pixel) that will turn on/off the carry flag
   jp c, dcm1_drawForeground   ;; [10] IF Carry, bit 7 was a 1, what means foreground color
   or  #00                     ;; [ 7] Bit7=0, draw next pixel of Background color
   .db #0xDA  ; JP C, xxxx     ;; [10] As carry is never set after an OR, this jump will never be done, and next instruction will be 3 bytes from here (effectively jumping over OR xx without a jump) 
dcm1_drawForeground:
   or  #00                     ;; [ 7] Bit7=1, draw next pixel of Foreground color
   rlca                        ;; [ 4] Rotate A 1 bit left to prepare it for inserting next pixel color (same 2 bits will be operated but, as long as A is rotated first, we effectively operate on next 2 bits to the right)
   djnz dcm1_nextpixel         ;; [13/8] IF B!=0, continue calculations with next pixel

   ld (de), a                  ;; [ 7] Save the 4 recently calculated pixels into video memory

   dec l                       ;; [ 4] L-- (1 byte less to finish this line)
   jp z, dcm1_endpixelline     ;; [10] If L=0, we have finished the line

   inc de                      ;; [ 6] ... and point to next 4 pixels in video memory (next byte, DE++)

   jp dcm1_next4pixels         ;; [10] Continue with next 4 pixels

dcm1_endpixelline:
   ;; Move to next pixel-line definition of the character
   pop hl                      ;; [10] Restore HL previous value from stack
   inc l                       ;; [ 4] Next pixel Line (characters are 8-byte-aligned in memory, so we only need to increment L, as H will not change)
   ld  a, l                    ;; [ 4] IF next pixel line corresponds to a new character (this is, we have finished drawing our character),
   and #0x07                   ;; [ 7] ... then L % 8 == 0, as it is 8-byte-aligned. 
   jp z, dcm1_end_printing     ;; [10] IF L%8 = 0, we have finished drawing the character, else, proceed to next line

   ;; Prepare to copy next line 
   ;;  -- Move DE pointer to the next pixel line on the video memory
   dec de                      ;; [ 6] DE-- : Restore DE to previous position in memory (it has moved 1 byte forward)
   ld  a, d                    ;; [ 4] Start of next pixel line normally is 0x0800 bytes away.
   add #0x08                   ;; [ 7]    so we add it to DE (just by adding 0x08 to D)
   ld  d, a                    ;; [ 4]
   and #0x38                   ;; [ 7] We check if we have crossed memory boundary (every 8 pixel lines)
   jp nz, dcm1_nextline        ;; [10]  by checking the 4 bits that identify present memory line. If 0, we have crossed boundaries
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
   ld   a,(cpct_mode_rom_status);; [13] A = mode_rom_status (present saved value)
   ld   b,  #GA_port_byte      ;; [ 7] B = Gate Array Port (0x7F)
   out (c), a                  ;; [12] GA Command: Set Video Mode and ROM status (100)
   ei                          ;; [ 4] Enable interrupts

   ret                         ;; [10] Return
