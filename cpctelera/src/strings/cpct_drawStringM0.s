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
.globl cpct_drawCharM0_asm

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_drawStringM0
;;
;;    Prints a null-terminated string with ROM characters on a given byte-aligned 
;; position on the screen in Mode 0 (160x200 px, 16 colours).
;;
;; C Definition:
;;    void <cpct_drawStringM0> (void* *string*, void* *video_memory*, <u8> *fg_pen*, <u8> *bg_pen*)
;;
;; Input Parameters (5 Bytes):
;;  (2B HL) string       - Pointer to the null terminated string being drawn
;;  (2B DE) video_memory - Video memory location where the string will be drawn
;;  (1B C ) fg_pen       - Foreground colour (PEN, 0-15)
;;  (1B B ) bg_pen       - Background colour (PEN, 0-15)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawStringM0_asm
;;
;; Parameter Restrictions:
;;  * *string* must be a null terminated string. It could contain any 8-bit value as 
;; characters except 0, which will signal the end of the string. Be careful to provide
;; strings with a 0 (null) at the end of the string. Otherwise, unexpected results may
;; happen (Typically, rubbish characters printed on screen and, occasionally, memory 
;; overwrite and even hangs or crashes).
;;  * *video_memory* could theoretically be any 16-bit memory location. It will work
;; outside current screen memory boundaries, which is useful if you use any kind of
;; double buffer. However, be careful where you use it, as it does no kind of check
;; or clipping, and it could overwrite data if you select a wrong place to draw.
;;  * *fg_pen* must be in the range [0-15]. It is used to access a colour mask table and,
;; so, a value greater than 15 will return a random colour mask giving unpredictable 
;; results (typically bad character rendering, with odd colour bars).
;;  * *bg_pen* must be in the range [0-15], with identical reasons to *fg_pen*.
;;
;; Requirements and limitations:
;;  * *Do not put this function's code below 0x4000 in memory*. In order to read
;; characters from ROM, this function enables Lower ROM (which is located 0x0000-0x3FFF),
;; so CPU would read code from ROM instead of RAM in first bank, effectively shadowing
;; this piece of code. This would lead to undefined results (typically program would
;; hang or crash).
;;  * This routine does not check for boundaries. If you draw too long strings or out 
;; of the screen, unpredictable results will happen.
;;  * Screen must be configured in Mode 0 (160x200 px, 16 colours)
;;  * This function requires the CPC *firmware* to be *DISABLED*. Otherwise, random
;; crashes might happen due to side effects.
;;  * This function *disables interrupts* during main loop (character printing), and
;; re-enables them at the end.
;;
;; Details:
;;    This function receives a null-terminated string and draws it to the screen in 
;; Mode 0 (160x200, 16 colours). This function calls <cpct_drawCharM0> to draw every    
;; character. *video_memory* parameter points to the byte where the string will be
;; drawn. The first pixel of that byte will be the upper-left corner of the string.
;; As this function uses a byte-pointer to refer to the upper-left corner of the 
;; string, it can only draw string on even-pixel columns (0, 2, 4, 6...), as 
;; every byte contains 2 pixels in Mode 0.
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    189 bytes (36 bytes this function, 153 bytes <cpct_drawCharM0>)
;;
;; Time Measures:
;; (start code)
;;   Case     |    Cycles    |   microSecs (us)
;; ----------------------------------------------
;;   Best     | 185 + 3774*L | 46.25 +  956.00*L
;;   Worst    | 185 + 4454*L | 46.25 + 1126.00*L
;; ----------------------------------------------
;; Asm saving |        -84   |          -21.00
;; ----------------------------------------------
;; (end code)
;;    L = Length of the string (excluding null-terminator character)
;;
;; These time measures take into account the time it takes to draw each individual
;; character (call to <cpct_drawCharM0>, assembly entry point).
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_drawStringM0::
   ;; Get parameters form stack
.if let_disable_interrupts_for_function_parameters
   ;; Way 1: Pop + Restoring SP. Faster, but consumes 5 bytes more, and requires disabling interrupts
   ld (drsm0_restoreSP+1), sp          ;; [20] Save SP into placeholder of the instruction LD SP, 0, to restore it later.
   di                                  ;; [ 4] Disable interrupts to ensure no one overwrites return address in the stack
   pop  af                             ;; [10] AF = Return Address
   pop  hl                             ;; [10] HL = Pointer to the null terminated string
   pop  de                             ;; [10] DE = Destination address (Video memory location where character will be printed)
   pop  bc                             ;; [10] BC = Colors (B=Background color, C=Foreground color) 
drsm0_restoreSP:
   ld sp, #0                           ;; [10] -- Restore Stack Pointer -- (0 is a placeholder which is filled up with 
                                       ;; .... actual SP value previously)
   ei                                  ;; [ 4] Enable interrupts again
.else 
   ;; Way 2: Pop + Push. Just 8 cycles more, but does not require disabling interrupts
   pop  af                             ;; [10] AF = Return Address
   pop  hl                             ;; [10] HL = Pointer to the null terminated string
   pop  de                             ;; [10] DE = Destination address (Video memory location where character will be printed)
   pop  bc                             ;; [10] BC = Colors (B=Background color, C=Foreground color) 
   push bc                             ;; [11] Restore Stack status pushing values again
   push de                             ;; [11] (Interrupt safe way, 8 cycles more)
   push hl                             ;; [11]
   push af                             ;; [11]
.endif

cpct_drawStringM0_asm::                ;; Assembly entry point

   ld (drsm0_values+1), bc             ;; [20] Save BC as LD direct value to be read later for saving colour values 
                                       ;; .... (Foreground and Background)
   jp drsm0_firstChar                  ;; [10] Jump to first char

drsm0_nextChar:
   push hl                             ;; [11] Save HL and DE to the stack before calling draw char
   push de                             ;; [11]
   call cpct_drawCharM0_asm            ;; [17] Draw next char
   pop  de                             ;; [10] Recover HL and DE from the stack
   pop  hl                             ;; [10]

drsm0_values:
   ld   bc, #00                        ;; [10] Restore BC value (Foreground and Background Colours)
   inc  de                             ;; [ 6] DE += 4 (point to next position in video memory, 8 pixels to the right)
   inc  de                             ;; [ 6]
   inc  de                             ;; [ 6]
   inc  de                             ;; [ 6]
   inc  hl                             ;; [ 6] HL += 1 (point to next character in the string)

drsm0_firstChar:
   ld  a, (hl)                         ;; [ 7] A = next character from the string
   or  a                               ;; [ 4] Check if A = 0
   jp  nz, drsm0_nextChar              ;; [10] if A != 0, A is next character, draw it, else end

drsm0_endString:
   ret                                 ;; [10] Return
