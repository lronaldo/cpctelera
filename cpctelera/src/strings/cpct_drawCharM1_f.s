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
;; Function: cpct_drawCharM1_f
;;
;;    Prints a ROM character on a given byte-aligned position on the screen 
;; in Mode 1 (320x200 px, 4 colours). It does it ~50% faster than <cpct_drawCharM1>.
;;
;; C Definition:
;;    void <cpct_drawCharM1_f> (void* *video_memory*, <u8> *fg_pen*, <u8> *bg_pen*, <u8> *ascii*)
;;
;; Input Parameters (5 Bytes):
;;  (2B DE) video_memory - Video memory location where the character will be drawn
;;  (1B C )  fg_pen       - Foreground palette colour index (Similar to BASIC's PEN, 0-3)
;;  (1B B )  bg_pen       - Background palette colour index (PEN, 0-3)
;;  (1B A )  ascii        - Character to be drawn (ASCII code)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawCharM1_f_asm
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
;; pixel columns (0, 4, 8, 12...), because each byte contains 4 pixels in Mode 0.
;; It prints the character in 2 colours (PENs) one for foreground (*fg_pen*), and 
;; the other for background (*bg_pen*). 
;;
;;    This function does the same as <cpct_drawCharM1>, but as fast  as possible, not taking 
;; into account any space constraints. It is unrolled, which makes it measure a great 
;; amount in bytes, but it is ~50% faster than <cpct_drawROMCharM1> The technique used 
;; to be so fast is difficult to understand: it uses dynamic code placement. I will 
;; try to sum up it here, and you can always read the detailed comments in the source 
;; to get a better understanding.
;;
;;    Basically, what this function does is what follows:
;;
;;  1 - It gets the 8-byte definitions of a character.               
;;  2 - It transforms each byte (a character line) into 2 bytes for video memory (8 pixels, 2 bits per pixel).
;;
;;    The trick is in transforming from 1-byte character-line definition to 2-bytes video 
;; memory colours. As we have only 4 colours per pixel, we have 4 possible transform 
;; operations either for foreground colour or for background. So, we have to do 4 
;; operations for each byte:
;;
;;  1 - Foreground colour for video byte 1
;;  2 - Background colour for video byte 1
;;  3 - Foreground colour for video byte 2
;;  4 - Background colour for video byte 2
;;
;;    What we do is, instead of adding branching logic to the inner loop that has to 
;; select the operation to do for each byte and type, we create 4 8-byte holes in 
;; the code that we call "dynamic code sections" (DCS). Then, we use logic at the 
;; start of the routine to select the 4 operations that are required, depending on
;; the selected foreground / background colours. When we know which operations are
;; to be performed, we fill in the holes (DCS) with the machine code that performs
;; the required operation. Then, when the inner loop is executed, it does not have
;; to do any branching operations, being much much faster.
;;
;;    The resulting code is very difficult to follow, and very big in size, but
;; when speed is the goal, this is the best approach.
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    349 bytes
;;
;; Time Measures:
;; (start code)
;;   Case     | Cycles | microSecs (us)
;; ------------------------------------
;;   Best     |  1952  |   488.00
;;   Worst    |  2670  |   668.50
;; ------------------------------------
;; Asm saving |   -80  |   -20
;; ------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_drawCharM1_f::
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

cpct_drawCharM1_f_asm::
   ld  (dcm1f_asciiHL+2), a    ;; [13] Save ASCII code of the character as data of a later "OR #data" instruction. 
                               ;; .... This is 1-cycle faster than pushing and popping to the stack and resets Carry Flag

   ;;----------------------------------------------------------------------------------------------
   ;; Set up foreground and background code that does the color mixing into bytes for video memory
   ;;----------------------------------------------------------------------------------------------

   ;;
   ;; ## PART 1 ## : Insert code for background colour generation
   ;;   This code inserts 2 pieces of dynamic code (Into dcm1f_start_b1bg and dcm1f_start_b2bg) that
   ;;   insert the background colour of each character line into the bytes that will be written to 
   ;;   video memory. The code inserted depends on the background colour: there are 4 possible pieces of
   ;;   code, so we use register B (background colour) to select the piece of code to insert.
   ;;
   inc   b                     ;; [ 4] Set Background colour between 1 and 4
   djnz dcm1f_bg01             ;; [13/8] If BGColor=00, do not jump, else, jump to BGColor=01 test

dcm1f_bg00: 
   ;; Background colour is 00. Insert Dynamic code into place-holders.
   ;;  *) This is the dynamic code that will be inserted into dcm1f_start_b(1/2)bg (3 bytes):
   ;;  Assembler    ; Mach.Code  Cycles
   ;;   XOR A       ; [ AF   ]   [ 4] A = 0 (As background is 0, no pixel is to be lightened up)
   ;;   JR  $+7     ; [ 1805 ]   [12] Jump to the end of the dynamic code section
   ld    a, #0xAF              ;; [ 7] A = AFh    (AF    = XOR A)
   ld (dcm1f_start_b2bg), a    ;; [13] <<= Insert XOR A into code for managing byte 2
   ld (dcm1f_start_b1bg), a    ;; [13] <<= Insert XOR A into code for managing byte 1
   ld   hl, #0x0518            ;; [10] HL = 0518h (18 05 = JR $+7)
   ld (dcm1f_start_b2bg+1), hl ;; [16] <<= Insert JR $+7 into code for managing byte 2
   ld (dcm1f_start_b1bg+1), hl ;; [16] <<= Insert JR $+7 into code for managing byte 1
   JP dcm1f_fg_dyncode         ;; [10] Finish background dynamic code inserting

dcm1f_bg01:
   ;; Background color is > 00 (so 01 to 11). All this 3 sections of code have at least 4 RRCA's
   ;; for one of the bytes, so we prepare HL for all of them now, avoiding useless repetitions
   ld   hl, #0x0F0F            ;; [10] HL = 0F0Fh (0F = RRCA)
   djnz dcm1f_bg10             ;; [13/8] If BGColor=01, do not jump, else, jump to BGColor=10 test

   ;; Background color is 01. Insert Dynamic code into place-holders.
   ;;  *) This is the dynamic code that will be inserted into dcm1f_start_b1bg (4 bytes):
   ;;  Assembler    ; Mach.Code  Cycles
   ;;   AND #0xF0   ; [ E6F0 ]    [ 7] Only first 4 bits are lightened, as this bits represent the bit 0 of 
   ;;                             .... the 4 first pixels (that has to be 1 for pixels activated)
   ;;   JR  $+6     ; [ 1804 ]    [12] Jump to the end of the dynamic code section
   ;;  *) And this is the dynamic code that will be inserted into dcm1f_start_b2bg (7 bytes):
   ;;  Assembler    ; Mach.Code  Cycles
   ;;   RRCA        ; [ 0F   ]    [ 4] Move low nibble of A to high nibble of A rotating bits
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- The 4 bits of the high nibble represent the bit 0 for the 4 pixels
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- And for colour 01 the bit 0 is the one that has to be active
   ;;   RRCA        ; [ 0F   ]    [ 4]
   ;;   AND #0xF0   ; [ E6F0 ]    [ 7] Set low nibble of A to 0 (as it represents bit 1 for our 4 pixels, 
   ;;                             .... and it should be always 0 for colour 01)   
   ;;   JR C, $xx   ; [ 38xx ]    [ 7] Create a conditional jump that will never be done (carry is always false). This is
   ;;                             .... only 7 cycles (2 NOP instructions would be 8, and we do not overwrite the last byte)
   ld (dcm1f_start_b2bg), hl   ;; [16] <<= Insert RRCA;RRCA;RRCA;RRCA into code for managing byte 2
   ld (dcm1f_start_b2bg+2), hl ;; [16]
   ld   hl, #0xF0E6            ;; [10] HL = F0E6h (E6 F0 = AND F0h)
   ld (dcm1f_start_b1bg), hl   ;; [16] <<= Insert AND #0xF0 into code for managing byte 1 and byte 2
   ld (dcm1f_start_b2bg+4), hl ;; [16]
   ld   hl, #0x0418            ;; [10] HL = 0418h (18 04 = JR $+6)
   ld (dcm1f_start_b1bg+2), hl ;; [16] <<= Insert JR $+6 into code for managing byte 1
   ld    a, #0x38              ;; [ 7] A = 38h (38 = JR C, xx)
   ld (dcm1f_start_b2bg+6), a  ;; [13] <<= Insert JR C, xx into code for managing byte 2
   jp dcm1f_fg_dyncode         ;; [10] Finish background dynamic code inserting
dcm1f_bg10:
   ;; Background color is > 01 (so 10 or 11). All this 2 sections of code have at least 4 RRCA's
   ;; in the management of the first byte. Then we insert it now, to avoid useless repetitions
   ld (dcm1f_start_b1bg), HL   ;; [16] <<= Insert RRCA;RRCA;RRCA;RRCA into code for managing byte 1
   ld (dcm1f_start_b1bg+2), HL ;; [16]
   djnz dcm1f_bg11             ;; [13/8] If BGColor=10, do not jump, else, jump to BGColor=11
   ;; Background color is 10. Insert Dynamic code into place-holders.
   ;;  *) This is the dynamic code that will be inserted into dcm1f_start_b1bg (7 bytes):
   ;;  Assembler    ; Mach.Code  Cycles
   ;;   RRCA        ; [ 0F   ]    [ 4] Move low nibble of A to high nibble of A rotating bits
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- The 4 bits of the high nibble represent the bit 0 for the 4 pixels
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- And for colour 01 the bit 0 is the one that has to be active
   ;;   RRCA        ; [ 0F   ]    [ 4]
   ;;   AND #0xF0   ; [ E6F0 ]    [ 7] Set low nibble of A to 0 (as it represents bit 1 for our 4 pixels, 
   ;;                             .... and it should be always 0 for colour 01)      
   ;;   JR C, $xx   ; [ 38xx ]    [ 7] Create a conditional jump that will never be done (carry is always false). This is 
   ;;                             .... only 7 cycles (2 NOP instructions would be 8, and we do not overwrite the last byte)
   ;;  *) And this is the dynamic code that will be inserted into dcm1f_start_b2bg (4 bytes):
   ;;  Assembler    ; Mach.Code  Cycles
   ;;   AND #0xF0   ; [ E6F0 ]    [ 7] Only first 4 bits are lightened, as this bits represent the bit 0 of 
   ;;                             .... the 4 first pixels (that has to be 1 for pixels activated)   
   ;;   JR  $+6     ; [ 1804 ]    [12] Jump to the end of the dynamic code section
   ld   hl, #0x0FE6            ;; [10] HL = 0FE6h (E6 0F = AND 0Fh)
   ld (dcm1f_start_b2bg), hl   ;; [16] <<= Insert AND 0x0F into code for managing bytes 1 and 2
   ld (dcm1f_start_b1bg+4), hl ;; [16]
   ld   hl, #0x0418            ;; [10] HL = 0418h (18 04 = JR $+6)
   ld (dcm1f_start_b2bg+2), hl ;; [16] <<= Insert JR $+6 into code for managing byte 2
   ld    a, #0x38              ;; [ 7] A = 38h (38 = JR C, xx)
   ld (dcm1f_start_b1bg+6), a  ;; [13] <<= Insert JR C, xx into code for managing byte 1
   jp dcm1f_fg_dyncode         ;; [10] Finish background dynamic code inserting
dcm1f_bg11:
   ;; Background color is 11. Insert Dynamic code into place-holders.
   ;;  *) This is the dynamic code that will be inserted into dcm1f_start_b(1/2)bg (8 bytes):
   ;;  Assembler    ; Mach.Code  Cycles
   ;;   RRCA        ; [ 0F   ]    [ 4] Interchange low and high nibble bits of A
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- We want to duplicate low / high nibble of A 
   ;;                             ....     (depending on which byte are we processing)
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- We have an inverted copy of A in C. By interchanging nibbles of A we can
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- mix A and C to get in A the same two nibbles, which is same value for 
   ;;                             ....     bits 0 and 1 of each pixel (colour 11)   
   ;;   XOR C       ; [ A9   ]    [ 4] Mix A and C to get the same values at the two nibbles of A
   ;;     OR #0xF0  ; [ F6F0 ] b1 [ 7]  -- This is only for byte 1 (dcm1f_start_b1bg)
   ;;     OR #0x0F  ; [ F60F ] b2 [ 7]  -- This is only for byte 2 (dcm1f_start_b2bg)
   ;;   XOR C       ; [ A9   ]    [ 4] ;; This last piece of code is static, as it never changes, so no need to insert it
   ld (dcm1f_start_b2bg), hl   ;; [16] <<= Insert RRCA;RRCA;RRCA;RRCA into code for managing byte 2
   ld (dcm1f_start_b2bg+2), hl ;; [16]
   ld  hl, #0xF6A9             ;; [10] HL = E6A9h (A9 = XOR C, F6 = OR xx (value in next byte))
   ld (dcm1f_start_b2bg+4), hl ;; [16] <<= Insert XOR C; OR xx into code for managing bytes 1 and 2
   ld (dcm1f_start_b1bg+4), hl ;; [16]
   ld   a, #0x0F               ;; [ 7] A = 0Fh (0F = Value for previous OR in second byte)
   ld (dcm1f_start_b2bg+6), a  ;; [13] <<= Insert 0F as immediate value for previous OR
   cpl                         ;; [ 4] A = F0h (F0 = Value for previous OR in first byte)
   ld (dcm1f_start_b1bg+6), a  ;; [13] <<= Insert F0 as immediate value for previous OR

   ;;
   ;; ## PART 2 ## : Insert code for foreground colour generation
   ;;   This code inserts 2 pieces of dynamic code into "dcm1f_start_b(1/2)fg" that manage the
   ;;   calculations for creating the foreground colour of each character line (2 bytes) from the 
   ;;   character pixel definition. As in part 1, the code inserted depends on the foreground colour,
   ;;   in register C, so we have 4 possible dynamic code sections to insert.
   ;;
dcm1f_fg_dyncode:
   ld    b, c                  ;; [ 4] B = Foreground colour
   inc   b                     ;; [ 4] Set Foreground colour between 1 and 4
   djnz dcm1f_fg01             ;; [13/8] If FGColor=00, do not jump, else, jump to FGColor=01 test
dcm1f_fg00:
   ;; Foreground color is 00. Insert Dynamic code into place-holders.
   ;;  *) This is the dynamic code that will be inserted into dcm1f_start_b(1/2)fg (3 bytes):
   ;;  Assembler    ; Mach.Code  Cycles
   ;;   XOR A       ; [ AF   ]   [ 4] A = 0 (As foreground is 0, no pixel is to be lightened up)
   ;;   JR  $+7     ; [ 1805 ]   [12] Jump to the end of the dynamic code section
   ld    a, #0xAF              ;; [ 7] A = AFh    (AF    = XOR A)
   ld (dcm1f_start_b2fg), a    ;; [13] <<= Insert XOR A into code for managing byte 2
   ld (dcm1f_start_b1fg), a    ;; [13] <<= Insert XOR A into code for managing byte 1
   ld   hl, #0x0518            ;; [10] HL = 0518h (18 05 = JR $+7)
   ld (dcm1f_start_b2fg+1), hl ;; [16] <<= Insert JR $+7 into code for managing byte 2
   ld (dcm1f_start_b1fg+1), hl ;; [16] <<= Insert JR $+7 into code for managing byte 1
   jp dcm1f_asciiHL            ;; [10] Finish foreground dynamic code inserting
dcm1f_fg01:
   ;; Foreground color is > 00 (so 01 to 11). All this 3 sections of code have at least 4 RRCA's
   ;; for one of the bytes, so we prepare HL for all of them now, avoiding useless repetitions
   ld   hl, #0x0F0F            ;; [10] HL = 0F0Fh (0F = RRCA)
   djnz dcm1f_fg10             ;; [13/8] If FGColor=01, do not jump, else, jump to FGColor=10 test
   ;; Foreground color is 01. Insert Dynamic code into place-holders.
   ;;  *) This is the dynamic code that will be inserted into dcm1f_start_b1fg (4 bytes):
   ;;  Assembler    ; Mach.Code  Cycles
   ;;   AND #0xF0   ; [ E6F0 ]    [ 7] Only first 4 bits are lightened, as this bits represent the bit 0 of 
   ;;                             .... the 4 first pixels (that has to be 1 for pixels activated)
   ;;   JR  $+6     ; [ 1804 ]    [12] Jump to the end of the dynamic code section
   ;;  *) And this is the dynamic code that will be inserted into dcm1f_start_b2bg (7 bytes):
   ;;  Assembler    ; Mach.Code  Cycles
   ;;   RRCA        ; [ 0F   ]    [ 4] Move low nibble of A to high nibble of A rotating bits
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- The 4 bits of the high nibble represent the bit 0 for the 4 pixels
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- And for colour 01 the bit 0 is the one that has to be active
   ;;   RRCA        ; [ 0F   ]    [ 4]
   ;;   AND #0xF0   ; [ E6F0 ]    [ 7] Set low nibble of A to 0 (as it represents bit 1 for our 4 pixels, 
   ;;                             .... and it should be always 0 for colour 01)   
   ;;   JR C, $xx   ; [ 38xx ]    [ 7] Create a conditional jump that will never be done (carry is always false). This is 
   ;;                             .... only 7 cycles (2 NOP instructions would be 8, and we do not overwrite the last byte)
   ld (dcm1f_start_b2fg), hl   ;; [16] <<= Insert RRCA;RRCA;RRCA;RRCA into code for managing byte 2
   ld (dcm1f_start_b2fg+2), hl ;; [16]
   ld   hl, #0xF0E6            ;; [10] HL = F0E6h (E6 F0 = AND F0h)
   ld (dcm1f_start_b1fg), hl   ;; [16] <<= Insert AND #0xF0 into code for managing byte 1 and byte 2
   ld (dcm1f_start_b2fg+4), hl ;; [16]
   ld   hl, #0x0418            ;; [10] HL = 0418h (18 04 = JR $+6)
   ld (dcm1f_start_b1fg+2), hl ;; [16] <<= Insert JR $+6 into code for managing byte 1
   ld    a, #0x38              ;; [ 7] A = 38h (38 = JR C, xx)
   ld (dcm1f_start_b2fg+6), a  ;; [13] <<= Insert JR C, xx into code for managing byte 2
   jp dcm1f_asciiHL            ;; [10] Finish background dynamic code inserting
dcm1f_fg10:
   ;; Background color is > 01 (so 10 or 11). All this 2 sections of code have at least 4 RRCA's
   ;; in the management of the first byte. Then we insert it now, to avoid useless repetitions
   ld (dcm1f_start_b1fg), hl   ;; [16] <<= Insert RRCA;RRCA;RRCA;RRCA into code for managing byte 1
   ld (dcm1f_start_b1fg+2), hl ;; [16]
   djnz dcm1f_fg11             ;; [13/8] If FGColor=10, do not jump, else, jump to FGColor=11
   ;; Foreground color is 10. Insert Dynamic code into place-holders.
   ;;  *) This is the dynamic code that will be inserted into dcm1f_start_b1fg (7 bytes):
   ;;  Assembler    ; Mach.Code  Cycles
   ;;   RRCA        ; [ 0F   ]    [ 4] Move low nibble of A to high nibble of A rotating bits
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- The 4 bits of the high nibble represent the bit 0 for the 4 pixels
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- And for colour 01 the bit 0 is the one that has to be active
   ;;   RRCA        ; [ 0F   ]    [ 4]
   ;;   AND #0xF0   ; [ E6F0 ]    [ 7] Set low nibble of A to 0 (as it represents bit 1 for our 4 pixels, 
   ;;                             .... and it should be always 0 for colour 01)
   ;;   JR C, $xx   ; [ 38xx ]    [ 7] Create a conditional jump that will never be done (carry is always false). This is
   ;;                             ....  only 7 cycles (2 NOP instructions would be 8, and we do not overwrite the last byte)
   ;;  *) And this is the dynamic code that will be inserted into dcm1f_start_b2fg (4 bytes):
   ;;  Assembler    ; Mach.Code  Cycles
   ;;   AND #0xF0   ; [ E6F0 ]    [ 7] Only first 4 bits are lightened, as this bits represent the bit 0 of the 4 
   ;;                             .... first pixels (that has to be 1 for pixels activated)
   ;;   JR  $+6     ; [ 1804 ]    [12] Jump to the end of the dynamic code section
   ld   hl, #0x0FE6            ;; [10] HL = 0FE6h (E6 0F = AND 0Fh)
   ld (dcm1f_start_b2fg), hl   ;; [16] <<= Insert AND 0x0F into code for managing bytes 1 and 2
   ld (dcm1f_start_b1fg+4), hl ;; [16]
   ld   hl, #0x0418            ;; [10] HL = 0418h (18 04 = JR $+6)
   ld (dcm1f_start_b2fg+2), hl ;; [16] <<= Insert JR $+6 into code for managing byte 2
   ld    a, #0x38              ;; [ 7] A = 38h (38 = JR C, xx)
   ld (dcm1f_start_b1fg+6), a  ;; [13] <<= Insert JR C, xx into code for managing byte 1
   jp dcm1f_asciiHL            ;; [10] Finish background dynamic code inserting
dcm1f_fg11:
   ;; Foreground color is 11. Insert Dynamic code into place-holders.
   ;;  *) This is the dynamic code that will be inserted into dcm1f_start_b(1/2)fg (8 bytes):
   ;;  Assembler    ; Mach.Code  Cycles
   ;;   RRCA        ; [ 0F   ]    [ 4] Interchange low and high nibble bits of A
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- We want to duplicate low / high nibble of A 
   ;;                             ....     (depending on which byte are we processing)
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- We have a copy of A in C. By interchanging nibbles of A we can
   ;;   RRCA        ; [ 0F   ]    [ 4]  -- mix A and C to get in A the same two nibbles, which is same value 
   ;;                             ....     for bits 0 and 1 of each pixel (colour 11)
   ;;   XOR C       ; [ A9   ]    [ 4] Mix A and C to get the same values at the two nibbles of A
   ;;    AND #0x0F  ; [ E60F ] b1 [ 7]  -- This is only for byte 1 (dcm1f_start_b1fg)
   ;;    AND #0xF0  ; [ E6F0 ] b2 [ 7]  -- This is only for byte 2 (dcm1f_start_b2fg)
   ;;   XOR C       ; [ A9   ]    [ 4] ;; This last piece of code is static, as it never changes, so no need to insert it
   ld (dcm1f_start_b2fg), hl   ;; [16] <<= Insert RRCA;RRCA;RRCA;RRCA into code for managing byte 2
   ld (dcm1f_start_b2fg+2), hl ;; [16]
   ld   hl, #0xE6A9            ;; [10] HL = E6A9h (A9 = XOR C, E6 = AND xx (value in next byte))
   ld (dcm1f_start_b2fg+4), hl ;; [16] <<= Insert XOR C; OR xx into code for managing bytes 1 and 2
   ld (dcm1f_start_b1fg+4), hl ;; [16]
   ld    a, #0xF0              ;; [ 7] A = F0h (F0 = Value for previous AND in second byte)
   ld (dcm1f_start_b2fg+6), a  ;; [13] <<= Insert F0 as immediate value for previous AND
   cpl                         ;; [ 4] A = 0Fh (0F = Value for previous AND in first byte)
   ld (dcm1f_start_b1fg+6), a  ;; [13] <<= Insert 0F as immediate value for previous AND

   ;;---------------------------------------------------------------------
   ;; Finished setting up dynamic code. Let's continue with the main task
   ;;---------------------------------------------------------------------

   ;; Make HL point to the starting byte of the desired character,
   ;; That is ==> HL = 8*(ASCII code) + char0_ROM_address 
dcm1f_asciiHL:
   xor   a                     ;; [ 4] A = 0
   or    #0                    ;; [ 7] A = ASCII Value and Resetting carry flag (#0 is a placeholder 
                               ;; .... that will be filled up with ASII value)
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

   ;;
   ;; Transform character definition into colour values for video memory and copy
   ;; them to the video position given at DE. Each character definition has 8 pixel-lines (8 bytes)
   ;; and each pixel line is to be transformed into 2 bytes for video memory (8 pixels, 2 bits per pixel)
   ;;
dcm1f_nextPixelLine: 
   ld    a, (hl)               ;; [ 7] C = Next Character pixel line definition (8 bits defining which pixels are on and off)

dcm1f_calculate_two_bytes:
   ;; Start calculations for the 1st byte
   ;;
   ld    c, a                  ;; [ 4] C = A :: C stores a copy of A that will be used to copy A nibbles up and down

   ;; DYNAMIC CODE SECTION
   ;;  * Processing foreground colour values of the 1st Byte
dcm1f_start_b1fg:
   .ds 7                       ;; [xx] This 7 bytes are filled up with dynamic code for calculating 
                               ;; .... foreground colour value inside 1st byte
   xor   c                     ;; [ 4] This operation is static, as it never changes no matter what
                               ;; ....  foreground colour is picked up

dcm1f_end_b1fg:
   ld    b, a                  ;; [ 4] Save calculated foreground colour value into B, 
                               ;; .... to mix it up later on with background colour
   ld    a, c                  ;; [ 4] Restore pixel-line definition into A
   cpl                         ;; [ 4] Invert pixel-line definition (light up background pixels instead of foreground ones)

   ;; DYNAMIC CODE SECTION
   ;;  * Processing background colour values of the 1st Byte
dcm1f_start_b1bg:
   .ds 7                       ;; [xx] This 7 bytes are filled up with dynamic code for calculating 
                               ;; .... background colour value inside 1st byte
   xor   c                     ;; [ 4] This operation is static, as it never changes no matter what
                               ;; ....  background colour is picked up

dcm1f_end_b1bg:
   ;; Save 1st Byte
   or    b                     ;; [ 4] Mix background and foreground colour values into A
   ld (de), a                  ;; [ 7] Store final calculated colour values into present video memory byte
   inc  de                     ;; [ 6] Point to the next video memory byte 

   ;; Start calculations for the 2nd byte
   ;;
   ld    a, c                  ;; [ 4] Restore pixel-line definition into A

   ;; DYNAMIC CODE SECTION
   ;;  * Processing foreground colour values of the 2nd Byte
dcm1f_start_b2fg:
   .ds 7                       ;; [xx] This 7 bytes are filled up with dynamic code for calculating 
                               ;; .... foreground colour value inside 2nd byte
   xor   c                     ;; [ 4] This operation is static, as it never changes no matter what
                               ;; ....  foreground colour is picked up

dcm1f_end_b2fg:
   ld    b, a                  ;; [ 4] Save calculated foreground colour value into B, 
                               ;; .... to mix it up later on with background colour
   ld    a, c                  ;; [ 4] Restore pixel-line definition into A
   cpl                         ;; [ 4] Invert pixel-line definition (light up background pixels instead of foreground ones)

   ;; DYNAMIC CODE SECTION
   ;;  * Processing background colour values of the 2nd Byte
dcm1f_start_b2bg:
   .ds 7                       ;; [xx] This 7 bytes are filled up with dynamic code for calculating 
                               ;; .... background colour value inside 2nd byte
   xor   c                     ;; [ 4] This operation is static, as it never changes no matter what
                               ;; ....  background colour is picked up

dcm1f_end_b2bg:
   ;; Save 2nd Byte
   or    b                     ;; [ 4] Mix background and foreground colour values into A
   ld (de), a                  ;; [ 7] Store final calculated colour values into present video memory byte

   ;; We have finished with this pixel-line, check if there are
   ;; more pixel-lines to continue or end drawing
   inc   l                     ;; [ 4] Next pixel Line (characters are 8-byte-aligned in memory, 
                               ;; .... so we only need to increment L, as H will not change)
   ld    a, l                  ;; [ 4] IF next pixel line corresponds to a new character 
                               ;; .... (this is, we have finished drawing our character),
   and   #0x07                 ;; [ 7] ... then L % 8 == 0, as it is 8-byte-aligned. 
   jp    z, dcm1f_end_printing ;; [10] IF L%8 = 0, we have finished drawing the character, else, proceed to next line

   ;; Prepare to copy next line 
   ;;  -- Move DE pointer to the next pixel line on the video memory
   dec  de                     ;; [ 6] DE-- : Restore DE to previous position in memory (it has moved 1 byte forward)
   ld    a, d                  ;; [ 4] Start of next pixel line normally is 0x0800 bytes away.
   ADD   #0x08                 ;; [ 7]    so we add it to DE (just by adding 0x08 to D)
   ld    d, a                  ;; [ 4]
   and   #0x38                 ;; [ 7] We check if we have crossed memory boundary (every 8 pixel lines)
   jp   nz, dcm1f_nextPixelLine;; [10]  by checking the 4 bits that identify present memory line. 
                               ;; .... If 0, we have crossed boundaries
dcm1f_8bit_boundary_crossed:
   ld    a, e                  ;; [ 4] DE = DE + C050h 
   ADD   #0x50                 ;; [ 7]   -- Relocate DE pointer to the start of the next pixel line 
   ld    e, a                  ;; [ 4]   -- in video memory
   ld    a, d                  ;; [ 4]
   ADC   #0xC0                 ;; [ 7]
   ld    d, a                  ;; [ 4]
   jp  dcm1f_nextPixelLine     ;; [10] Jump to continue with next pixel line

dcm1f_end_printing:
   ;; After finishing character printing, restore previous ROM and Interrupts status
   ld    a, (_cpct_mode_rom_status);; [13] A = mode_rom_status (present saved value)
   ld    b, #GA_port_byte      ;; [ 7] B = Gate Array Port (0x7F)
   out (c), a                  ;; [12] GA Command: Set Video Mode and ROM status (100)
   ei                          ;; [ 4] Enable interrupts

   ret                         ;; [10] Return
