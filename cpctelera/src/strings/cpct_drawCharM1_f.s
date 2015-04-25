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
;## FUNCTION: _cpct_drawROMCharM1_fast                                ###
;########################################################################
;### This function does the same as _cpct_drawROMCharM1, but as fast  ###
;### as possible, not taking into account any space constraints. It   ###
;### is unrolled, which makes it measure a great amount in bytes, but ###
;### it is 60% faster than _cpct_drawROMCharM1.                       ###
;### The technique used to be so fast is difficult to understant: it  ###
;### uses dynamic code placement. I will try to sum up it here, and   ###
;### you can always read the detailed comments in the source to get a ###
;### better understanding.                                            ###
;### Basically, what this function does is what follows:              ###
;###  1. It gets the 8-byte definitions of a character.               ###
;###  2. It transforms each byte (a character line) into 2 bytes for  ###
;###     video memory (8 pixels, 2 bits per pixel).                   ###
;### The trick is in transforming from 1-byte character-line defini-  ###
;### tion to 2-bytes video memory colors. As we have only 4 colors    ###
;### per pixel, we have 4 possible transform operations either for    ###
;### foreground color or for background. So, we have to do 4 operati- ###
;### ons for each byte:                                               ###
;###  1. Foreground color for video byte 1                            ###
;###  2. Background color for video byte 1                            ###
;###  3. Foreground color for video byte 2                            ###
;###  4. Background color for video byte 2                            ###
;### What we do is,instead of adding banching logic to the inner loop ###
;### that has to select the operation to do for each byte and type,   ###
;### we create 4 8-byte holes in the code that we call "dynamic code  ###
;### sections" (DCS). Then, we use logic at the start of the routine  ###
;### to select the 4 operations that are required, depending on the   ###
;### selected foreground/background colors. When we know which opera- ###
;### tions are to be performed, we fill in the holes (DCS) with the   ###
;### machine code that performes the required operation. Then, when   ###
;### the inner loop is executed, it does not have to do any branching ###
;### operations, being much much faster.                              ###
;### The resulting code is very difficult to follow, and very big in  ###
;### size, but when speed is the goal, this is the best approach.     ###
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
;### MEMORY: 347 bytes                                                ###
;### TIME:                                                            ###
;###  Best case  = 1966 cycles (491,50 us)                            ###
;###  Worst case = 2684 cycles (672,00 us)                            ###
;########################################################################
;

_cpct_drawCharM1_f::
   ;; GET Parameters from the stack 
.if let_disable_interrupts_for_function_parameters
   ;; Way 1: Pop + Restoring SP. Faster, but consumes 7 bytes more, and requires disabling interrupts
   LD (dcm1f_restoreSP+1), SP  ;; [20] Save SP into placeholder of the instruction LD SP, 0, to quickly restore it later.
   DI                          ;; [ 4] Disable interrupts to ensure no one overwrites return address in the stack
   POP  AF                     ;; [10] AF = Return Address
   POP  DE                     ;; [10] DE = Destination address (Video memory location where character will be printed)
   POP  BC                     ;; [10] BC = Colors (B=Background color, C=Foreground color) 
   POP  HL                     ;; [10] HL = ASCII code of the character (H=0, L=ASCII code)
dcm1f_restoreSP:
   LD SP, #0                   ;; [10] -- Restore Stack Pointer -- (0 is a placeholder which is filled up with actual SP value previously)
   EI                          ;; [ 4] Enable interrupts again
.else 
   ;; Way 2: Pop + Push. Just 6 cycles more, but does not require disabling interrupts
   pop  af                     ;; [10] AF = Return Address
   pop  de                     ;; [10] DE = Destination address (Video memory location where character will be printed)
   pop  bc                     ;; [10] BC = Colors (B=Background color, C=Foreground color) 
   pop  hl                     ;; [10] HL = ASCII code of the character (H=0, L=ASCII code)
   push hl                     ;; [11] Restore Stack status pushing values again
   push bc                     ;; [11] (Interrupt safe way, 6 cycles more)
   push de                     ;; [11]
   push af                     ;; [11]
.endif

   ;; Save ASCII code of the character to get it later
   LD  A, L                    ;; [ 4] A = ASCII code of the character

_cpct_drawCharM1_f_asm::
   LD  (dcm1f_asciiHL+1), A    ;; [13] Save ASCII code of the character as data of a later "LD HL, #data" instruction. This is faster than pushing and popping to the stack because H needs to be resetted

   ;;----------------------------------------------------------------------------------------------
   ;; Set up foreground and background code that does the color mixing into bytes for video memory
   ;;----------------------------------------------------------------------------------------------

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
   ;; Background color is 10. Insert Dynamic code into placeholders.
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
   LD (dcm1f_start_b2bg), HL   ;; [16] <<= Insert AND 0x0F into code for managing bytes 1 and 2
   LD (dcm1f_start_b1bg+4), HL ;; [16]
   LD  HL, #0x0418             ;; [10] HL = 0418h (18 04 = JR $+6)
   LD (dcm1f_start_b2bg+2), HL ;; [16] <<= Insert JR $+6 into code for managing byte 2
   LD   A, #0x38               ;; [ 7] A = 38h (38 = JR C, xx)
   LD (dcm1f_start_b1bg+6), A  ;; [13] <<= Insert JR C, xx into code for managing byte 1
   JP dcm1f_fg_dyncode         ;; [10] Finish background dynamic code inserting
dcm1f_bg11:
   ;; Background color is 11. Insert Dynamic code into placeholders.
   ;;  *) This is the dynamic code that will be inserted into dcm1f_start_b(1/2)bg (8 bytes):
   ;;  Assembler    ; Mach.Code  Cycles
   ;;   RRCA        ; [ 0F   ]    [ 4] Interchange low and high nibble bits of A
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- We want to duplicate low/high nibble of A (depending on which byte are we processing)
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- We have an inverted copy of A in C. By interchanging nibbles of A we can
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
   LD   A, #0x0F               ;; [ 7] A = 0Fh (0F = Value for previous OR in second byte)
   LD (dcm1f_start_b2bg+6), A  ;; [13] <<= Insert 0F as inmediate value for previous OR
   CPL                         ;; [ 4] A = F0h (F0 = Value for previous OR in first byte)
   LD (dcm1f_start_b1bg+6), A  ;; [13] <<= Insert F0 as inmediate value for previous OR

   ;;
   ;; ## PART 2 ## : Insert code for foreground color generation
   ;;   This code inserts 2 pieces of dynamic code into "dcm1f_start_b(1/2)fg" that manage the
   ;;   calculations for creating the foreground color of each character line (2 bytes) from the 
   ;;   character pixel definition. As in part 1, the code inserted depends on the foreground color,
   ;;   in register C, so we have 4 possible dynamic code sections to insert.
   ;;
dcm1f_fg_dyncode:
   LD  B, C                    ;; [ 4] B = Foreground color
   INC B                       ;; [ 4] Set Foreground color between 1 and 4
   DJNZ dcm1f_fg01             ;; [13/8] If FGColor=00, do not jump, else, jump to FGColor=01 test
dcm1f_fg00:
   ;; Foreground color is 00. Insert Dynamic code into placeholders.
   ;;  *) This is the dynamic code that will be inserted into dcm1f_start_b(1/2)fg (3 bytes):
   ;;  Assembler    ; Mach.Code  Cycles
   ;;   XOR A       ; [ AF   ]   [ 4] A = 0 (As foreground is 0, no pixel is to be lightened up)
   ;;   JR  $+7     ; [ 1805 ]   [12] Jump to the end of the dynamic code section
   LD  A, #0xAF                ;; [ 7] A = AFh    (AF    = XOR A)
   LD (dcm1f_start_b2fg), A    ;; [13] <<= Insert XOR A into code for managing byte 2
   LD (dcm1f_start_b1fg), A    ;; [13] <<= Insert XOR A into code for managing byte 1
   LD  HL, #0x0518             ;; [10] HL = 0518h (18 05 = JR $+7)
   LD (dcm1f_start_b2fg+1), HL ;; [16] <<= Insert JR $+7 into code for managing byte 2
   LD (dcm1f_start_b1fg+1), HL ;; [16] <<= Insert JR $+7 into code for managing byte 1
   JP dcm1f_asciiHL            ;; [10] Finish foreground dynamic code inserting
dcm1f_fg01:
   ;; Foreground color is > 00 (so 01 to 11). All this 3 sections of code have at least 4 RRCA's
   ;; for one of the bytes, so we prepare HL for all of them now, avoiding unuseful repetitions
   LD  HL, #0x0F0F             ;; [10] HL = 0F0Fh (0F = RRCA)
   DJNZ dcm1f_fg10             ;; [13/8] If FGColor=01, do not jump, else, jump to FGColor=10 test
   ;; Foreground color is 01. Insert Dynamic code into placeholders.
   ;;  *) This is the dynamic code that will be inserted into dcm1f_start_b1fg (4 bytes):
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
   LD (dcm1f_start_b2fg), HL   ;; [16] <<= Insert RRCA;RRCA;RRCA;RRCA into code for managing byte 2
   LD (dcm1f_start_b2fg+2), HL ;; [16]
   LD  HL, #0xF0E6             ;; [10] HL = F0E6h (E6 F0 = AND F0h)
   LD (dcm1f_start_b1fg), HL   ;; [16] <<= Insert AND #0xF0 into code for managing byte 1 and byte 2
   LD (dcm1f_start_b2fg+4), HL ;; [16]
   LD  HL, #0x0418             ;; [10] HL = 0418h (18 04 = JR $+6)
   LD (dcm1f_start_b1fg+2), HL ;; [16] <<= Insert JR $+6 into code for managing byte 1
   LD   A, #0x38               ;; [ 7] A = 38h (38 = JR C, xx)
   LD (dcm1f_start_b2fg+6), A  ;; [13] <<= Insert JR C, xx into code for managing byte 2
   JP dcm1f_asciiHL            ;; [10] Finish background dynamic code inserting
dcm1f_fg10:
   ;; Background color is > 01 (so 10 or 11). All this 2 sections of code have at least 4 RRCA's
   ;; in the management of the first byte. Then we insert it now, to avoid unuseful repetitions
   LD (dcm1f_start_b1fg), HL   ;; [16] <<= Insert RRCA;RRCA;RRCA;RRCA into code for managing byte 1
   LD (dcm1f_start_b1fg+2), HL ;; [16]
   DJNZ dcm1f_fg11             ;; [13/8] If FGColor=10, do not jump, else, jump to FGColor=11
   ;; Foreground color is 10. Insert Dynamic code into placeholders.
   ;;  *) This is the dynamic code that will be inserted into dcm1f_start_b1fg (7 bytes):
   ;;  Assembler    ; Mach.Code  Cycles
   ;;   RRCA        ; [ 0F   ]    [ 4] Move low nibble of A to high nibble of A rotating bits
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- The 4 bits of the high nibble represent the bit 0 for the 4 pixels
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- And for color 01 the bit 0 is the one that has to be active
   ;;   RRCA        ; [ 0F   ]    [ 4]
   ;;   AND #0xF0   ; [ E6F0 ]    [ 7] Set low nibble of A to 0 (as it represents bit 1 for our 4 pixels, an it should be always 0 for color 01)
   ;;   JR C, $xx   ; [ 38xx ]    [ 7] Create a conditional jump that will never be done (carry is always false). This is only 7 cycles (2 NOP instructions would be 8, and we do not overwritte the last byte)
   ;;  *) And this is the dynamic code that will be inserted into dcm1f_start_b2fg (4 bytes):
   ;;  Assembler    ; Mach.Code  Cycles
   ;;   AND #0xF0   ; [ E6F0 ]    [ 7] Only first 4 bits are lightened, as this bits represent the bit 0 of the 4 first pixels (that has to be 1 for pixels activated)
   ;;   JR  $+6     ; [ 1804 ]    [12] Jump to the end of the dynamic code section
   LD  HL, #0x0FE6             ;; [10] HL = 0FE6h (E6 0F = AND 0Fh)
   LD (dcm1f_start_b2fg), HL   ;; [16] <<= Insert AND 0x0F into code for managing bytes 1 and 2
   LD (dcm1f_start_b1fg+4), HL ;; [16]
   LD  HL, #0x0418             ;; [10] HL = 0418h (18 04 = JR $+6)
   LD (dcm1f_start_b2fg+2), HL ;; [16] <<= Insert JR $+6 into code for managing byte 2
   LD   A, #0x38               ;; [ 7] A = 38h (38 = JR C, xx)
   LD (dcm1f_start_b1fg+6), A  ;; [13] <<= Insert JR C, xx into code for managing byte 1
   JP dcm1f_asciiHL            ;; [10] Finish background dynamic code inserting
dcm1f_fg11:
   ;; Foreground color is 11. Insert Dynamic code into placeholders.
   ;;  *) This is the dynamic code that will be inserted into dcm1f_start_b(1/2)fg (8 bytes):
   ;;  Assembler    ; Mach.Code  Cycles
   ;;   RRCA        ; [ 0F   ]    [ 4] Interchange low and high nibble bits of A
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- We want to duplicate low/high nibble of A (depending on which byte are we processing)
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- We have a copy of A in C. By interchanging nibbles of A we can
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- mix A and C to get in A the same two nibbles, which is same value for bits 0 and 1 of each pixel (color 11)
   ;;   XOR C       ; [ A9   ]    [ 4] Mix A and C to get the same values at the two nibbles of A
   ;;    AND #0x0F  ; [ E60F ] b1 [ 7]  -- This is only for byte 1 (dcm1f_start_b1fg)
   ;;    AND #0xF0  ; [ E6F0 ] b2 [ 7]  -- This is only for byte 2 (dcm1f_start_b2fg)
   ;;   XOR C       ; [ A9   ]    [ 4] ;; This last piece of code is static, as it never changes, so no need to insert it
   LD (dcm1f_start_b2fg), HL   ;; [16] <<= Insert RRCA;RRCA;RRCA;RRCA into code for managing byte 2
   LD (dcm1f_start_b2fg+2), HL ;; [16]
   LD  HL, #0xE6A9             ;; [10] HL = E6A9h (A9 = XOR C, E6 = AND xx (value in next byte))
   LD (dcm1f_start_b2fg+4), HL ;; [16] <<= Insert XOR C; OR xx into code for managing bytes 1 and 2
   LD (dcm1f_start_b1fg+4), HL ;; [16]
   LD   A, #0xF0               ;; [ 7] A = F0h (F0 = Value for previous AND in second byte)
   LD (dcm1f_start_b2fg+6), A  ;; [13] <<= Insert F0 as inmediate value for previous AND
   CPL                         ;; [ 4] A = 0Fh (0F = Value for previous AND in first byte)
   LD (dcm1f_start_b1fg+6), A  ;; [13] <<= Insert 0F as inmediate value for previous AND

   ;;---------------------------------------------------------------------
   ;; Finished setting up dynamic code. Let's continue with the main task
   ;;---------------------------------------------------------------------

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
   LD   A,(cpct_mode_rom_status);; [13] A = mode_rom_status (present value)
   AND  #0b11111011            ;; [ 7] bit 3 of A = 0 --> Lower ROM enabled (0 means enabled)
   LD   B, #GA_port_byte       ;; [ 7] B = Gate Array Port (0x7F)
   DI                          ;; [ 4] Disable interrupts to prevent firmware from taking control while Lower ROM is enabled
   OUT (C), A                  ;; [12] GA Command: Set Video Mode and ROM status (100)

   ;;
   ;; Transform character definition into color values for video memory and copy
   ;; them to the video position given at DE. Each character definition has 8 pixel-lines (8 bytes)
   ;; and each pixel line is to be transformed into 2 bytes for video memory (8 pixels, 2 bits per pixel)
   ;;
dcm1f_nextPixelLine: 
   LD  A, (HL)                 ;; [ 7] C = Next Character pixel line definition (8 bits defining which pixels are on and off)

dcm1f_calculate_two_bytes:
   ;; Start calculations for the 1st byte
   ;;
   LD  C, A                    ;; [ 4] C = A :: C stores a copy of A that will be used to copy A nibbles up and down

   ;; DYNAMIC CODE SECTION
   ;;  * Processing foreground color values of the 1st Byte
dcm1f_start_b1fg:
   .ds 7                       ;; [xx] This 7 bytes are filled up with dynamic code for calculating foreground color value inside 1st byte
   XOR C                       ;; [ 4] This operation is static, as it never changes no matter what foreground color is picked up

dcm1f_end_b1fg:
   LD  B, A                    ;; [ 4] Save calculated foreground color value into B, to mix it up later on with background color
   LD  A, C                    ;; [ 4] Restore pixel-line definition into A
   CPL                         ;; [ 4] Invert pixel-line definition (light up background pixels instead of foreground ones)

   ;; DYNAMIC CODE SECTION
   ;;  * Processing background color values of the 1st Byte
dcm1f_start_b1bg:
   .ds 7                       ;; [xx] This 7 bytes are filled up with dynamic code for calculating background color value inside 1st byte
   XOR C                       ;; [ 4] This operation is static, as it never changes no matter what background color is picked up

dcm1f_end_b1bg:
   ;; Save 1st Byte
   OR  B                       ;; [ 4] Mix background and foreground color values into A
   LD  (DE), A                 ;; [ 7] Store final calculated color values into present video memory byte
   INC DE                      ;; [ 6] Point to the next video memory byte 

   ;; Start calculations for the 2nd byte
   ;;
   LD  A, C                    ;; [ 4] Restore pixel-line definition into A

   ;; DYNAMIC CODE SECTION
   ;;  * Processing foreground color values of the 2nd Byte
dcm1f_start_b2fg:
   .ds 7                       ;; [xx] This 7 bytes are filled up with dynamic code for calculating foreground color value inside 2nd byte
   XOR C                       ;; [ 4] This operation is static, as it never changes no matter what foreground color is picked up

dcm1f_end_b2fg:
   LD  B, A                    ;; [ 4] Save calculated foreground color value into B, to mix it up later on with background color
   LD  A, C                    ;; [ 4] Restore pixel-line definition into A
   CPL                         ;; [ 4] Invert pixel-line definition (light up background pixels instead of foreground ones)

   ;; DYNAMIC CODE SECTION
   ;;  * Processing background color values of the 2nd Byte
dcm1f_start_b2bg:
   .ds 7                       ;; [xx] This 7 bytes are filled up with dynamic code for calculating background color value inside 2nd byte
   XOR C                       ;; [ 4] This operation is static, as it never changes no matter what background color is picked up

dcm1f_end_b2bg:
   ;; Save 2nd Byte
   OR  B                       ;; [ 4] Mix background and foreground color values into A
   LD  (DE), A                 ;; [ 7] Store final calculated color values into present video memory byte

   ;; We have finished with this pixel-line, check if there are
   ;; more pixel-lines to continue or end drawing
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
   LD   A,(cpct_mode_rom_status);; [13] A = mode_rom_status (present saved value)
   LD   B,  #GA_port_byte      ;; [ 7] B = Gate Array Port (0x7F)
   OUT (C), A                  ;; [12] GA Command: Set Video Mode and ROM status (100)
   EI                          ;; [ 4] Enable interrupts

   RET                         ;; [10] Return
