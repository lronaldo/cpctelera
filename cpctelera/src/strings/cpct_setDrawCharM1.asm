;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2018 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_strings

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_setDrawCharM1
;;
;;    Sets foreground and background colours that will be used by <cpct_drawCharM1_inner>
;; when called. <cpct_drawCharM1_inner> is used by both <cpct_drawCharM1> and <cpct_drawStringM1>.
;;
;; C Definition:
;;    void <cpct_setDrawCharM1> (<u8> *fg_pen*, <u8> *bg_pen*) __z88dk_callee
;;
;; Input Parameters (2 Bytes):
;;  (1B E )  fg_pen       - Foreground palette colour index (Similar to BASIC's PEN, 0-3)
;;  (1B D )  bg_pen       - Background palette colour index (PEN, 0-3)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_setDrawCharM1_asm
;;
;; Parameter Restrictions:
;;  * *fg_pen* must be in the range [0-3]. It is used to access a colour mask table and,
;; so, a value greater than 3 will return a random colour mask giving unpredictable 
;; results (typically bad character rendering, with odd colour bars).
;;  * *bg_pen* must be in the range [0-3], with identical reasons to *fg_pen*.
;;
;; Requirements and limitations:
;;  * This function *will not work from ROM*, as it uses self-modifying code.
;;
;; Details:
;;    This function sets the internal values of <cpct_drawCharM1_inner>, so that next calls
;; to <cpct_drawCharM1> or <cpct_drawStringM1> will use these new values. Concretely, these
;; values are *fg_pen* (foreground colour) and *bg_pen* (background colour). This function
;; receives 2 colour values in the range [0-3] and transforms them into the 16 possible 
;; combinations of these 2 colours in groups of 4 pixels. Namely, 
;; (start code)
;;                       ________________________________________________________
;;  4-bit combinations  | 0000 | 0001 | 0010 | 0011 | 0100 | 0101 | 0110 | 0111 |
;;  Displayed pixels    | bbbb | bbbF | bbFb | bbFF | bFbb | bFbF | bFFb | bFFF |
;;                      |---------------------------|---------------------------|
;;  4-bit combinations  | 1000 | 1001 | 1010 | 1011 | 1100 | 1101 | 1110 | 1111 |
;;  Displayed pixels    | Fbbb | FbbF | FbFb | FbFF | FFbb | FFbF | FFFb | FFFF |
;;                      |---------------------------|---------------------------|
;;                           * b = pixel coloured using bg_pen (background)
;;                           * F = pixel coloured using fg_pen (foreground)
;; (end code)
;;    In mode 1, each of these 16 possible combinations corresponds to 1 byte in screen pixel 
;; format that encodes the 2 selected colours.
;;
;;    These 16 combinations are stored into an internal array called *cpct_char2pxM1*, which is
;; used by <cpct_drawCharM1_inner> to draw characters on the screen in mode 1, 4-pixels at a time.
;; This array remains constant unless a new call to <cpct_setDrawCharM1> changes it. This lets
;; the user change colours once and use them many times for subsequent calls to draw functions.
;;
;;    The appropriate use of this function is to call it each time a new pair of colours is
;; required for following character drawings to be made either with <cpct_drawCharM1> or with
;; <cpct_drawStringM1>. If the same colours are to be used for many drawings, a single call to 
;; <cpct_setDrawCharM1> followed by many drawing calls would be the best practice. 
;;
;;    You may found use examples consulting documentation for <cpct_drawCharM1> and <cpct_drawStringM1>.
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    ASM bindings  - 81 bytes ( +80 cpct_drawCharM1_inner +4 dc_mode1_ct table = 165 bytes)
;;      C bindings  - 84 bytes ( +80 cpct_drawCharM1_inner +4 dc_mode1_ct table = 168 bytes)
;;
;; Time Measures:
;; (start code)
;;   Case     | microSecs (us) | CPU Cycles
;; -------------------------------------------
;;   Any      |      185       |     740
;; -------------------------------------------
;; Asm saving |      -10       |     -40
;; -------------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Global symbols
;;
.globl dc_mode1_ct      ;; Colour conversion table (4 values to convert PEN values [0-3] to their screen pixel format)
.globl cpct_char2pxM1   ;; Screen Pixel Format conversion table (16 pairs of pixels BG-BG-BG-BG, BG-BG-BG-FG, BG-BG-FG-BG...., FG-FG-FG-FG)

   ;; Get Colour values converted to Screen Pixel Format
   ;; B = bg colours :: C = fg colours
   ;;
   ld    hl, #dc_mode1_ct     ;; [3] HL = Pointer to the start of the Conversion Colour Table
   ld     a, d                ;; [1] A  = bg_pen  (save the value for later use)
   ld     d, #0               ;; [2] DE = fg_pen  (D=0, E=fg_pen)
   add   hl, de               ;; [3] HL += fg_pen (So that HL points to the converted value for fg_pen)
   ld     c, (hl)             ;; [2] C = fg pixel colour values (4 pixels of foreground colour)
   ld    hl, #dc_mode1_ct     ;; [3] HL = Pointer to the start of the Conversion Colour Table
   ld     e, a                ;; [1] DE = bg_pen  (D=0, E=bg_pen)
   add   hl, de               ;; [3] HL += bg_pen (So that HL points to the converted value for bg_pen)
   ld     b, (hl)             ;; [2] B = bg pixel colour values (4 pixels of background colour)
     
   ;; Now generate the 16 possible combinations of 2 colours (FG=Foreground, BG=Background)
   ;; and 4 pixels per byte. These will be stored in the table cpct_char2pxM1 that drawing
   ;; functions (cpct_drawCharM1 & cpct_drawStringM1) will use to draw faster.

   ;; HL Points to the start of the cpct_char2pxM1 table that needs to be filled 
   ld    hl, #cpct_char2pxM1  ;; [3] HL = Address of the first byte of the table (byte 0)

   ;; Combination: BG-BG-BG-BG
   ld  (hl), b                ;; [2] B already holds 4 pixels of background colour, so save it directly to (HL)
   inc   hl                   ;; [2] Make HL point to table byte 1

   ;; This loop contains 7 combinations that will be inserted twice just changing the first
   ;; pixel colour from BackGround to ForeGround before repeating. That will insert 14 combinations
   ;; into the table with half the amount of bytes in the generating code. It also includes the 
   ;; combination FG-BG-BG-BG that will be inserted only once, making a total of 15 combinations.
   ld     a, b    ;; [1] A = BG-BG-BG-BG combination
loop:
   ;; Insert pixel combinations in the table
   ld    de, #0xEEDD ;; [3] D = 0xEE [1110|1110], E = 0xDD [1101|1101]
                     ;;    Two combinations used 6 times for AND operations. 
                     ;;    Having them into registers saves cycles and bytes.

   ;; Combination: [BG/FG]-BG-BG-FG
                  ;; A = [BG/FG]-BG-BG-BG combination
   xor    c       ;; [1] / Make 4th pixel FG (rightmost bit of each nibble)
   and    d       ;; [1] | 3 leftmost bits of each nibble remain the same (3 first pixels) 0xEE = [1110|1110]
   xor    c       ;; [1] \ D = 0xEE, C = 4 foreground pixel values
   ld  (hl), a    ;; [2] Store the [BG/FG]-BG-BG-FG combination at the byte [1 or 9] of the table
   inc   hl       ;; [2] HL points to the byte [2 or 10] of the table

   ;; Combination: [BG/FG]-BG-FG-BG
   ld     a, b    ;; [1] A = [BG/FG]-BG-BG-BG
   xor    c       ;; [1] / Make 3rd pixel FG (Insert FG colour here > [--F-|--F-])
   and    e       ;; [1] | Insert FG Colour in the 3rd bit of each nibble: 0xDD = [1101|1101]
   xor    c       ;; [1] \ E = 0xDD, C = 4 foreground pixel values  
   ld  (hl), a    ;; [2] Store the [BG/FG]-BG-FG-BG combination at the byte [2 or 10] of the table
   inc   hl       ;; [2] HL points to the byte [3 or 11] of the table

   ;; Combination: [BG/FG]-BG-FG-FG 
                  ;; A = [BG/FG]-BG-FG-BG
   xor    c       ;; [1] / Make 4th pixel FG (rightmost bit of each nibble)
   and    d       ;; [1] | 3 leftmost bits of each nibble remain the same (3 first pixels) 0xEE = [1110|1110]
   xor    c       ;; [1] \ D = 0xEE, C = 4 foreground pixel values
   ld  (hl), a    ;; [2] Store the [BG/FG]-BG-FG-FG combination at the byte [3 or 11] of the table
   inc   hl       ;; [2] HL points to the byte [4 or 12] of the table

   ;; Combination: [BG/FG]-FG-BG-BG
   ld     a, b    ;; [1] A = [BG/FG]-BG-BG-BG
   xor    c       ;; [1] / Make 2nd pixel FG (Insert FG colour here > [-F--|-F--])
   and   #0xBB    ;; [2] | Insert FG Colour in the 2nd bit of each nibble: 0xCC = [1011|1011]
   xor    c       ;; [1] \ C = 4 foreground pixel values
   ld  (hl), a    ;; [2] Store the [BG/FG]-FG-BG-BG combination at the byte [4 or 12] of the table
   inc   hl       ;; [2] HL points to the byte [5 or 13] of the table

   ;; Combination: [BG/FG]-FG-BG-FG
                  ;; A = [BG/FG]-FG-BG-BG
   xor    c       ;; [1] / Make 4th pixel FG (rightmost bit of each nibble)
   and    d       ;; [1] | 3 leftmost bits of each nibble remain the same (3 first pixels) 0xEE = [1110|1110]
   xor    c       ;; [1] \ D = 0xEE, C = 4 foreground pixel values
   ld  (hl), a    ;; [2] Store the [BG/FG]-FG-BG-FG combination at the byte [5 or 13] of the table 
   inc   hl       ;; [2] HL points to the byte [6 or 14] of the table

   ;; Combination: [BG/FG]-FG-FG-BG
   ld     a, b    ;; [1] A = [BG/FG]-BG-BG-BG
   xor    c       ;; [1] / Make 2nd and 3rd pixels FG (Insert FG colour here > [-FF-|-FF-])
   and   #0x99    ;; [2] | Insert FG Colour in the 2nd and 3rd bits of each nibble: 0x99 = [1001|1001]
   xor    c       ;; [1] \ C = 4 foreground pixel values
   ld  (hl), a    ;; [2] Store the [BG/FG]-FG-FG-BG combination at the byte [6 or 14] of the table  
   inc   hl       ;; [2] HL points to the byte [7 or 15] of the table

   ;; Combination: [BG/FG]-FG-FG-FG
                  ;; A = [BG/FG]-FG-FG-BG
   xor    c       ;; [1] / Make 4th pixel FG (rightmost bit of each nibble)
   and    d       ;; [1] | 3 leftmost bits of each nibble remain the same (3 first pixels) 0xEE = [1110|1110]
   xor    c       ;; [1] \ D = 0xEE, C = 4 foreground pixel values
   ld  (hl), a    ;; [2] Store the [BG/FG]-FG-FG-FG combination at the byte [7 or 15] of the table 

   ;; Update Return instruction: This instruction will be a NOP (0x00) the first
   ;; iteration of this loop, and a RET (0xC9) the second interaction. That will save
   ;; as a register, as no counter will be required. The only thing to do is to
   ;; XOR the instruction against 0xC9 to make it switch between 0x00 and 0xC9.
   ld    de, #nopret    ;; [3] DE = Memory address where the NOP/RET instruction is stored
   ld     a, (de)       ;; [2] A = Present value of the NOP/RET instruction
   xor   #0xC9          ;; [2] XOR the NOP/RET instruction against 0xC9 to make it switch from NOP to RET and viceversa
   ld  (de), a          ;; [2] Store the switched version of the NOP/RET instruction

   ;; Do nothing the first iteration, return on the second
nopret = .
   ret            ;; [1/3] NOP / RET Placeholder

   ;; If execution arrives here, means that previous instruction was a NOP,
   ;; therefore this is the first iteration of the loop (on the second, previous instruction will be a RET)
   ;; We only need to increment HL now (not in the second iteration), because we have filled half the table
   inc   hl       ;; [2] HL points to the byte 8 of the table
   
   ;; Combination: FG-BG-BG-BG
   ld     a, b    ;; [1] A = BG-BG-BG-BG
   xor    c       ;; [1] / Make 1st pixels FG (Insert FG colour here > [F---|F---])
   and   #0x77    ;; [2] | Insert FG Colour in the 1st bits of each nibble: 0x77 = [0111|0111]
   xor    c       ;; [1] \ C = 4 foreground pixel values
   ld  (hl), a    ;; [2] Store the FG-BG-BG-BG combination at the byte 8 of the table 
   inc   hl       ;; [2] HL points to the byte 9 of the table
   ld     b, a    ;; [1] B = FG-BG-BG-BG (New combination to be used on the second iteration of the loop)

   jr    loop     ;; [3] Repeat the loop for a second time (FG-XX-XX-XX combinations)
